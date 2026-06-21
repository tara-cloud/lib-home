#include "emo_face.h"
#include <Arduino.h>

// ─── Shared display pointer ───────────────────────────────────────────────────
static IDisplay* _d = nullptr;

// ─── Eye geometry (128×64 canvas) ────────────────────────────────────────────
// Two large rounded-rect eyes, upper-centre, symmetrical.
// Inward tilt = inner top-corner of each eye is TILT px lower than outer.
static const int L_EX  = 10;    // left eye left edge
static const int R_EX  = 74;    // right eye left edge
static const int EYE_Y = 16;    // eye top edge (upper-centre area)
static const int EYE_W = 42;    // wide — expressive
static const int EYE_H = 28;    // tall — curiosity
static const int EYE_R = 9;     // corner radius — very rounded
static const int TILT  = 4;     // px — inner-top-corner tilt depth

// Centre coords for non-idle expressions (reuse old helpers)
static const int L_CX  = L_EX + EYE_W / 2;
static const int R_CX  = R_EX + EYE_W / 2;
static const int L_CY  = EYE_Y + EYE_H / 2;
static const int R_CY  = EYE_Y + EYE_H / 2;

// ─── Idle animation state ─────────────────────────────────────────────────────
static int           _eyeDX       = 0;
static int           _eyeDY       = 0;

static bool          _blinking    = false;
static unsigned long _blinkStart  = 0;
static unsigned long _nextBlinkMs = 3500;

static unsigned long _lastMoveMs  = 0;
static unsigned long _nextMoveMs  = 2500;

static unsigned long _nextBlinkInterval() {
    return 3000UL + (unsigned long)random(0, 4001);  // 3–7 s
}

// ─── drawFace() ───────────────────────────────────────────────────────────────
// Renders both eyes with a given lid-close fraction (0.0 open → 1.0 closed).
// Applies current micro-offset. Inward tilt masked via pixel-exact steps.

static void drawFace(float lidFrac) {
    _d->clear();
    _d->fillScreen(false);   // black background

    for (int side = 0; side < 2; side++) {
        int ex = constrain((side == 0 ? L_EX : R_EX) + _eyeDX, 0, 128 - EYE_W);
        int ey = constrain(EYE_Y + _eyeDY, 0, 64 - EYE_H);

        if (lidFrac >= 0.98f) {
            // Fully closed — one thick horizontal line
            _d->fillRect(ex, ey + EYE_H / 2 - 1, EYE_W, 3, true);
            continue;
        }

        int openH = max(2, (int)(EYE_H * (1.0f - lidFrac)));
        int topY  = ey + (EYE_H - openH);

        // White filled rounded rect
        _d->fillRoundRect(ex, topY, EYE_W, openH, EYE_R, true);

        // Inward tilt — mask stepped triangle at inner-top corner
        if (lidFrac < 0.6f) {
            for (int t = 0; t < TILT; t++) {
                int maskW = TILT - t;
                if (side == 0)   // left eye — mask top-right
                    _d->fillRect(ex + EYE_W - maskW, topY + t, maskW, 1, false);
                else             // right eye — mask top-left
                    _d->fillRect(ex, topY + t, maskW, 1, false);
            }
        }
    }

    _d->show();
}

// ─── blinkAnimation() ─────────────────────────────────────────────────────────
// Smooth 3-frame close + hold + 3-frame open. Total ≈ 360 ms. Non-blocking.

static void blinkAnimation(unsigned long now) {
    unsigned long e = now - _blinkStart;

    float lid;
    if      (e < 60)  lid = e / 60.0f;
    else if (e < 180) lid = 1.0f;                              // hold closed
    else if (e < 360) lid = 1.0f - (float)(e - 180) / 180.0f; // reopen
    else              lid = 0.0f;

    drawFace(lid);

    if (e >= 360) {
        _blinking    = false;
        _nextBlinkMs = now + _nextBlinkInterval();
    }
}

// ─── updateEyes() ─────────────────────────────────────────────────────────────
// Randomly shifts gaze ±3–5 px every 2–5 s. Centre-weighted. Non-blocking.

static void updateEyes(unsigned long now) {
    if (now - _lastMoveMs < _nextMoveMs) return;

    static const int8_t moves[][2] = {
        { 0,  0}, { 0,  0}, { 0,  0},   // rest (3× weight)
        {-5,  0}, { 5,  0},              // look left / right
        { 0, -3}, { 0,  3},              // look up / down
        {-4, -2}, { 4, -2},              // up-left / up-right
        {-3,  2}, { 3,  2},              // down-left / down-right
    };
    int idx = random(0, (int)(sizeof(moves) / sizeof(moves[0])));
    _eyeDX = moves[idx][0];
    _eyeDY = moves[idx][1];

    _lastMoveMs = now;
    _nextMoveMs = 2000UL + (unsigned long)random(0, 3001);

    drawFace(0.0f);
}

// ─── idleBehavior() ───────────────────────────────────────────────────────────
// Called every loop() tick while FACE_IDLE.
// Priority: blink > micro-move. Only one update per call.

static void idleBehavior() {
    unsigned long now = millis();

    if (!_blinking && now >= _nextBlinkMs) {
        _blinking   = true;
        _blinkStart = now;
    }

    if (_blinking) {
        blinkAnimation(now);
        return;
    }

    updateEyes(now);
}

// ─── Shared drawing helpers (used by non-idle expressions) ────────────────────

static void _eye(int cx, int cy, float lidFrac, int dx, int dy) {
    int ex = cx - EYE_W/2, ey = cy - EYE_H/2;
    _d->fillRoundRect(ex, ey, EYE_W, EYE_H, EYE_R, true);
    if (lidFrac > 0.0f) {
        int lh = (int)(EYE_H * lidFrac) + 1;
        _d->fillRect(ex + 1, ey, EYE_W - 2, lh, false);
        _d->drawHLine(ex + 1, ey + lh, EYE_W - 2, true);
    }
}

static void _eyes(float lid, int dx = 0, int dy = 0) {
    _eye(L_CX + dx, L_CY + dy, lid, 0, 0);
    _eye(R_CX + dx, R_CY + dy, lid, 0, 0);
}

static void _smile(int cx, int cy, int w, int d) {
    _d->drawHLine(cx - w/2, cy, w, true);
    for (int i = 1; i <= d; i++) {
        _d->drawPixel(cx - w/2 + i - 1, cy + i, true);
        _d->drawPixel(cx + w/2 - i,     cy + i, true);
    }
}

static void _frown(int cx, int cy, int w, int d) {
    _d->drawHLine(cx - w/2, cy, w, true);
    for (int i = 1; i <= d; i++) {
        _d->drawPixel(cx - w/2 + i - 1, cy - i, true);
        _d->drawPixel(cx + w/2 - i,     cy - i, true);
    }
}

static void _oval(int cx, int cy, int w, int h) {
    _d->drawRoundRect(cx - w/2, cy - h/2, w, h, h/2, true);
    _d->fillRoundRect(cx - w/2 + 1, cy - h/2 + 1, w - 2, h - 2, h/2 - 1, false);
}

static void _brow(int cx, int cy, int yOff, int tilt) {
    int bx = cx - 14, by = cy + yOff, bw = 28, bh = 3;
    if (tilt == 0) { _d->fillRoundRect(bx, by, bw, bh, 1, true); return; }
    int mid = bw / 2;
    _d->fillRect(bx,       by + (tilt > 0 ? -tilt : 0), mid, bh, true);
    _d->fillRect(bx + mid, by + (tilt < 0 ?  tilt : 0), mid, bh, true);
}

// ─── Expression functions ─────────────────────────────────────────────────────

static void _faceWaitingConfig() {
    _d->clear();
    _d->drawHLine(14, 10, 16, true); _d->drawHLine(14, 11, 16, true);
    for (int i = 0; i < 16; i++) _d->drawPixel(66 + i, 13 - (i * 4) / 16, true);
    _d->drawCircle(22, 28, 9, true); _d->fillCircle(22, 28, 4, true);
    _d->fillCircle(74, 32, 6, true);
    _d->drawHLine(36, 50, 8, true); _d->drawHLine(44, 46, 8, true);
    _d->drawHLine(52, 50, 8, true); _d->drawHLine(60, 46, 8, true);
    _d->setTextSize(1); _d->setTextColor(true);
    _d->setCursor(104, 5); _d->print("?");
    _d->show();
}

static void _faceListening() {
    _d->clear();
    _eyes(0.0f);
    for (int r = 6; r <= 14; r += 4) {
        _d->drawCircle(118, 32, r, true);
        _d->fillRect(104, 32 - r - 1, r + 2, r * 2 + 2, false);
    }
    _oval(64, 55, 14, 10);
    _d->show();
}

static void _faceThinking() {
    _d->clear();
    _eye(L_CX, L_CY, 0.0f, 3, -4); _eye(R_CX, R_CY, 0.0f, 3, -7);
    _brow(R_CX, R_CY, -16, 0);
    for (int i = 0; i < 3; i++) _d->fillCircle(52 + i * 10, 56, 2 + i, true);
    _d->show();
}

static void _faceSpeaking() {
    _d->clear();
    _eyes(0.0f);
    _oval(64, 54, 28, 14);
    _d->show();
}

static void _faceSleeping() {
    _d->clear();
    for (int ex : {L_EX, R_EX}) {
        _d->drawHLine(ex + 2, EYE_Y + EYE_H/2,     EYE_W - 4, true);
        _d->drawHLine(ex + 4, EYE_Y + EYE_H/2 + 1, EYE_W - 8, true);
    }
    _d->setTextSize(1); _d->setTextColor(true);
    _d->setCursor(98, 12); _d->print("z");
    _d->setCursor(104, 6); _d->print("Z");
    _d->setCursor(112, 1); _d->print("Z");
    _d->show();
}

static void _faceError() {
    _d->clear();
    for (int cx : {L_CX, R_CX}) {
        for (int di = -7; di <= 7; di++) {
            _d->drawPixel(cx + di, L_CY + di, true);
            _d->drawPixel(cx + di, L_CY - di, true);
        }
    }
    _frown(64, 57, 22, 3);
    _d->drawHLine(0, 62, 128, true);
    _d->setTextSize(1); _d->setTextColor(true);
    _d->setCursor(45, 2); _d->print("ERROR");
    _d->show();
}

// ─── Factory ──────────────────────────────────────────────────────────────────

FaceRenderer emo_face_renderer(IDisplay* display) {
    _d           = display;
    _blinking    = false;
    _nextBlinkMs = 3500;
    _eyeDX = _eyeDY = 0;
    _lastMoveMs  = 0;
    _nextMoveMs  = 2500;

    FaceRenderer r = {};
    r.render[FACE_BOOTING]        = nullptr;
    r.render[FACE_CONNECTING]     = _faceWaitingConfig;
    r.render[FACE_REGISTERING]    = _faceWaitingConfig;
    r.render[FACE_WAITING_CONFIG] = _faceWaitingConfig;
    r.render[FACE_CONFIGURING]    = _faceThinking;
    r.render[FACE_IDLE]           = idleBehavior;
    r.render[FACE_LISTENING]      = _faceListening;
    r.render[FACE_THINKING]       = _faceThinking;
    r.render[FACE_SPEAKING]       = _faceSpeaking;
    r.render[FACE_SLEEPING]       = _faceSleeping;
    r.render[FACE_ERROR]          = _faceError;
    return r;
}
