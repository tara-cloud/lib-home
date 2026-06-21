#include "tara_face.h"
#include "tara_eyelids.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

TaraFace::TaraFace(IDisplay* display, int screenW, int screenH, uint8_t fps)
    : _d(display), _sw(screenW), _sh(screenH) {
    setFramerate(fps);
    _resetToDefaults();
}

void TaraFace::_resetToDefaults() {
    eyeW = eyeWt = eyeWDefault;
    eyeH = eyeHt = eyeHDefault;
    eyeR = eyeRDefault;
    space = spaceDef;

    float totalW = eyeWDefault * 2 + spaceDef;
    leftX  = leftXt = (_sw - totalW) / 2.0f;
    leftY  = leftYt = (_sh - eyeHDefault) / 2.0f;
    rightX = leftX + eyeWDefault + spaceDef;
    rightY = leftY;

    _sweatDrops.init(leftX, leftY, (int)eyeWDefault);
}

// ─── begin ────────────────────────────────────────────────────────────────────

void TaraFace::begin() {
    blinkTimer = millis() + _randomMs(blinkIntervalSec, blinkVarSec);
    idleTimer  = millis() + _randomMs(idleIntervalSec,  idleVarSec);

    FaceRenderer r = {};
    r.render[FACE_BOOTING]        = nullptr;
    r.render[FACE_CONNECTING]     = [this]() { setPosition(TARA_CENTER);              update(); };
    r.render[FACE_REGISTERING]    = [this]() { setMood(TARA_DEFAULT);                 update(); };
    r.render[FACE_WAITING_CONFIG] = [this]() { setMood(TARA_TIRED);                   update(); };
    r.render[FACE_CONFIGURING]    = [this]() { setMood(TARA_DEFAULT);                 update(); };
    r.render[FACE_IDLE]           = [this]() { setMood(TARA_DEFAULT);                 update(); };
    r.render[FACE_LISTENING]      = [this]() { setMood(TARA_HAPPY);                   update(); };
    r.render[FACE_THINKING]       = [this]() { setPosition(TARA_NE);                  update(); };
    r.render[FACE_SPEAKING]       = [this]() { anim_laugh();                          update(); };
    r.render[FACE_SLEEPING]       = [this]() { setMood(TARA_TIRED); close();          update(); };
    r.render[FACE_ERROR]          = [this]() { setMood(TARA_ANGRY); anim_confused();  update(); };
    face_register(r);
}

// ─── Eye shape ────────────────────────────────────────────────────────────────

void TaraFace::setWidth(uint8_t left, uint8_t right)        { eyeWDefault = left; eyeWt = left; }
void TaraFace::setHeight(uint8_t left, uint8_t right)       { eyeHDefault = left; eyeHt = left; }
void TaraFace::setBorderRadius(uint8_t left, uint8_t right) { eyeRDefault = left; eyeR  = left; }
void TaraFace::setSpaceBetween(int s)                       { spaceDef = s; space = s; }

// ─── Mood ─────────────────────────────────────────────────────────────────────

void TaraFace::setMood(uint8_t mood) {
    _lids.setMood(mood, eyeHDefault / 2.0f);
}

// ─── Position ─────────────────────────────────────────────────────────────────

void TaraFace::setPosition(uint8_t pos) {
    float cx = (_sw - (eyeWDefault * 2 + spaceDef)) / 2.0f;
    float cy = (_sh - eyeHDefault) / 2.0f;
    int maxX = getScreenConstraintX();
    int maxY = getScreenConstraintY();

    switch (pos) {
        case TARA_N:  leftXt = cx;    leftYt = 0;    break;
        case TARA_NE: leftXt = maxX;  leftYt = 0;    break;
        case TARA_E:  leftXt = maxX;  leftYt = cy;   break;
        case TARA_SE: leftXt = maxX;  leftYt = maxY; break;
        case TARA_S:  leftXt = cx;    leftYt = maxY; break;
        case TARA_SW: leftXt = 0;     leftYt = maxY; break;
        case TARA_W:  leftXt = 0;     leftYt = cy;   break;
        case TARA_NW: leftXt = 0;     leftYt = 0;    break;
        default:      leftXt = cx;    leftYt = cy;   break;
    }
}

// ─── Curiosity / cyclops ──────────────────────────────────────────────────────

void TaraFace::setCuriosity(bool on) { curious = on; }

void TaraFace::setCyclops(bool on) {
    cyclops = on;
    eyeWt = on ? 0 : eyeWDefault;
    eyeHt = on ? 0 : eyeHDefault;
}

// ─── Blink ────────────────────────────────────────────────────────────────────

void TaraFace::close(bool left, bool right) { if (left || right) eyeHt = 1; }
void TaraFace::open(bool left, bool right)  { if (left || right) eyeHt = eyeHDefault; }
void TaraFace::blink(bool left, bool right) { close(left, right); open(left, right); }
void TaraFace::close() { close(true, true); }
void TaraFace::open()  { open(true, true); }
void TaraFace::blink() { blink(true, true); }

// ─── Automated animations ────────────────────────────────────────────────────

void TaraFace::setAutoblinker(bool on, int intervalSec, int variationSec) {
    autoBlink = on;
    blinkIntervalSec = intervalSec;
    blinkVarSec = variationSec;
    if (on) blinkTimer = millis() + _randomMs(intervalSec, variationSec);
}

void TaraFace::setIdleMode(bool on, int intervalSec, int variationSec) {
    idleMode = on;
    idleIntervalSec = intervalSec;
    idleVarSec = variationSec;
    if (on) idleTimer = millis() + _randomMs(intervalSec, variationSec);
}

// ─── Effects ──────────────────────────────────────────────────────────────────

void TaraFace::setHFlicker(bool on, uint8_t amp) { hFlicker = on; hFlickerAmp = amp; }
void TaraFace::setVFlicker(bool on, uint8_t amp) { vFlicker = on; vFlickerAmp = amp; }
void TaraFace::setSweat(bool on) { sweat = on; }

// ─── One-shot animations ──────────────────────────────────────────────────────

void TaraFace::anim_confused() { if (!confused) { confused = true; confusedAt = millis(); } }
void TaraFace::anim_laugh()    { if (!laughing)  { laughing  = true; laughAt   = millis(); } }

// ─── Frame rate / constraints ─────────────────────────────────────────────────

void TaraFace::setFramerate(uint8_t fps) { frameMs = fps > 0 ? 1000 / fps : 20; }
int  TaraFace::getScreenConstraintX()   { return _sw - (int)(eyeWDefault * 2) - spaceDef; }
int  TaraFace::getScreenConstraintY()   { return _sh - (int)eyeHDefault; }

// ─── Helpers ──────────────────────────────────────────────────────────────────

float TaraFace::_tween(float cur, float tgt)     { return (cur + tgt) / 2.0f; }
unsigned long TaraFace::_randomMs(int base, int var) {
    return (unsigned long)(base + random(0, var + 1)) * 1000UL;
}

// ─── update / drawEyes ────────────────────────────────────────────────────────

void TaraFace::update() {
    if (millis() - lastFrameMs < (unsigned long)frameMs) return;
    lastFrameMs = millis();
    drawEyes();
}

void TaraFace::drawEyes() {
    unsigned long now = millis();

    // ── Step 1: one-shot animations ───────────────────────────────────────────
    if (confused) {
        hFlicker = true; hFlickerAmp = 20;
        if (now - confusedAt >= CONFUSED_DUR) { confused = false; hFlicker = false; }
    }
    if (laughing) {
        vFlicker = true; vFlickerAmp = 5;
        if (now - laughAt >= LAUGH_DUR) { laughing = false; vFlicker = false; }
    }

    // ── Step 2: automated events ──────────────────────────────────────────────
    if (autoBlink && now >= blinkTimer) {
        blink();
        blinkTimer = now + _randomMs(blinkIntervalSec, blinkVarSec);
    }
    if (idleMode && now >= idleTimer) {
        leftXt = random(0, getScreenConstraintX() + 1);
        leftYt = random(0, getScreenConstraintY() + 1);
        idleTimer = now + _randomMs(idleIntervalSec, idleVarSec);
    }

    // ── Step 3: flicker offsets ───────────────────────────────────────────────
    int hOff = 0, vOff = 0;
    if (hFlicker) { hFlickerAlt = !hFlickerAlt; hOff = hFlickerAlt ?  hFlickerAmp : -hFlickerAmp; }
    if (vFlicker) { vFlickerAlt = !vFlickerAlt; vOff = vFlickerAlt ?  vFlickerAmp : -vFlickerAmp; }

    // ── Step 4: tween all geometry ────────────────────────────────────────────
    eyeW   = _tween(eyeW,  eyeWt);
    eyeH   = _tween(eyeH,  eyeHt);
    leftX  = _tween(leftX, leftXt);
    leftY  = _tween(leftY, leftYt);
    rightX = leftX + eyeW + space;
    rightY = leftY;

    // Curiosity — outer eye grows taller when looking far sideways
    float lExtra = 0, rExtra = 0;
    if (curious) {
        if (leftX  <= 10)                         lExtra = 8;
        if (!cyclops && rightX >= _sw - eyeW - 10) rExtra = 8;
    }

    _lids.tick();   // tween eyelid heights (defined in tara_mood.h)

    // ── Step 5: draw ──────────────────────────────────────────────────────────
    _d->clear();
    _d->fillScreen(false);

    // Left eye
    int lx = (int)leftX  + hOff,  ly = (int)(leftY  - lExtra / 2) + vOff;
    int lw = (int)eyeW,            lh = (int)(eyeH + lExtra);
    if (lw > 0 && lh > 0) {
        _d->fillRoundRect(lx, ly, lw, lh, (int)eyeR, true);
        drawEyelids(_d, lx, ly, lw, lh, true,
                    _lids.tired, _lids.angry, _lids.happy,
                    (int)eyeHDefault, (int)eyeR);
    }

    // Right eye (hidden in cyclops mode)
    int rx = (int)rightX + hOff,  ry = (int)(rightY - rExtra / 2) + vOff;
    int rw = cyclops ? 0 : (int)eyeW,  rh = cyclops ? 0 : (int)(eyeH + rExtra);
    if (rw > 0 && rh > 0) {
        _d->fillRoundRect(rx, ry, rw, rh, (int)eyeR, true);
        drawEyelids(_d, rx, ry, rw, rh, false,
                    _lids.tired, _lids.angry, _lids.happy,
                    (int)eyeHDefault, (int)eyeR);
    }

    if (sweat) _sweatDrops.tick(_d, leftY + eyeH);

    _d->show();
}
