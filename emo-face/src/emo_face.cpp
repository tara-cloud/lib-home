#include "emo_face.h"
#include <Arduino.h>

// ─── Idle animation state (module-level, shared across calls) ─────────────────
static IDisplay*     _d            = nullptr;
static unsigned long _nextBlinkMs  = 0;
static unsigned long _lastAnimMs   = 0;
static bool          _isBlinking   = false;
static int           _pupilDX      = 0;
static int           _pupilDY      = 0;
static int           _driftStep    = 0;

// ─── Layout constants (128×64 canvas) ────────────────────────────────────────
static const int L_CX = 38, L_CY = 32;   // left eye centre
static const int R_CX = 90, R_CY = 32;   // right eye centre
static const int EW   = 30, EH = 22, ER = 6; // eye w, h, corner-radius

// ─── Helpers ──────────────────────────────────────────────────────────────────

static unsigned long _nextBlink() { return 3000UL + (unsigned long)random(0, 3000); }

static void _eye(int cx, int cy, float lidFrac, int dx, int dy) {
    int ex = cx - EW/2, ey = cy - EH/2;
    _d->drawRoundRect(ex, ey, EW, EH, ER, true);
    if (lidFrac > 0.0f) {
        int lh = (int)(EH * lidFrac) + 1;
        _d->fillRect(ex + 1, ey, EW - 2, lh, false);
        _d->drawHLine(ex + 1, ey + lh, EW - 2, true);
    }
    int pw = 10, ph = 10;
    int px = constrain(cx - pw/2 + dx, ex + 3, ex + EW - pw - 3);
    int py = constrain(cy - ph/2 + dy, ey + 3, ey + EH - ph - 3);
    _d->fillRoundRect(px, py, pw, ph, 3, true);
    _d->fillCircle(px + pw - 3, py + 2, 2, false);
}

static void _eyes(float lid, int dx = 0, int dy = 0) {
    _eye(L_CX, L_CY, lid, dx, dy);
    _eye(R_CX, R_CY, lid, dx, dy);
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

static void _faceIdle() {
    unsigned long now = millis();

    // Blink trigger
    if (!_isBlinking && now >= _nextBlinkMs) {
        _isBlinking = true;
        _lastAnimMs = now;
    }
    if (_isBlinking) {
        unsigned long e = now - _lastAnimMs;
        float lid = (e < 60)  ? (float)e / 60.0f
                  : (e < 120) ? 1.0f
                  : (e < 180) ? 1.0f - (float)(e - 120) / 60.0f : 0.0f;
        if (e >= 180) { _isBlinking = false; _nextBlinkMs = now + _nextBlink(); lid = 0.0f; }
        _d->clear(); _eyes(lid, _pupilDX, _pupilDY); _d->show();
        return;
    }
    // Subtle pupil drift every 3 s
    if (now - _lastAnimMs > 3000) {
        static const int drifts[][2] = {{-2,0},{2,0},{0,-1},{0,1},{0,0}};
        _driftStep = (_driftStep + 1) % 5;
        _pupilDX = drifts[_driftStep][0];
        _pupilDY = drifts[_driftStep][1];
        _lastAnimMs = now;
        _d->clear(); _eyes(0.0f, _pupilDX, _pupilDY); _d->show();
    }
}

static void _faceWaitingConfig() {
    _d->clear();
    // Asymmetric brows
    _d->drawHLine(14, 10, 16, true); _d->drawHLine(14, 11, 16, true);
    for (int i = 0; i < 16; i++) _d->drawPixel(66 + i, 13 - (i * 4) / 16, true);
    // Eyes — left open, right squinted
    _d->drawCircle(22, 28, 9, true); _d->fillCircle(22, 28, 4, true);
    _d->fillCircle(74, 32, 6, true);
    // Wavy mouth
    _d->drawHLine(36, 50, 8, true); _d->drawHLine(44, 46, 8, true);
    _d->drawHLine(52, 50, 8, true); _d->drawHLine(60, 46, 8, true);
    _d->setTextSize(1); _d->setTextColor(true);
    _d->setCursor(104, 5); _d->print("?");
    _d->show();
}

static void _faceHappy() {
    _d->clear();
    // Squinted eyes with cheek blush
    _eye(L_CX, L_CY, 0.0f, 0, -2); _eye(R_CX, R_CY, 0.0f, 0, -2);
    for (int i = 0; i < 3; i++) { _d->fillCircle(18 + i*5, 46, 2, true); _d->fillCircle(100 + i*5, 46, 2, true); }
    _smile(64, 54, 32, 5);
    _d->show();
}

static void _faceSad() {
    _d->clear();
    _eyes(0.15f, 0, 3);
    _brow(L_CX, L_CY, -14, -3); _brow(R_CX, R_CY, -14, 3);
    _frown(64, 56, 26, 4);
    _d->fillCircle(L_CX + 8, L_CY + 14, 3, true); _d->fillCircle(L_CX + 8, L_CY + 10, 2, true);
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
    _eyes(0.0f, _pupilDX, _pupilDY);
    _oval(64, 54, 28, 14);
    _d->show();
}

static void _faceSleeping() {
    _d->clear();
    for (int cx : {L_CX, R_CX}) {
        _d->drawHLine(cx - EW/2 + 2, L_CY, EW - 4, true);
        _d->drawHLine(cx - EW/2 + 4, L_CY + 1, EW - 8, true);
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
            _d->drawPixel(cx + di, R_CY + di, true);
            _d->drawPixel(cx + di, R_CY - di, true);
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
    _nextBlinkMs = 3000;
    _lastAnimMs  = 0;

    FaceRenderer r = {};
    r.render[FACE_BOOTING]       = nullptr;          // no face during boot
    r.render[FACE_CONNECTING]    = _faceWaitingConfig;
    r.render[FACE_REGISTERING]   = _faceWaitingConfig;
    r.render[FACE_WAITING_CONFIG]= _faceWaitingConfig;
    r.render[FACE_CONFIGURING]   = _faceThinking;
    r.render[FACE_IDLE]          = _faceIdle;
    r.render[FACE_LISTENING]     = _faceListening;
    r.render[FACE_THINKING]      = _faceThinking;
    r.render[FACE_SPEAKING]      = _faceSpeaking;
    r.render[FACE_SLEEPING]      = _faceSleeping;
    r.render[FACE_ERROR]         = _faceError;
    return r;
}
