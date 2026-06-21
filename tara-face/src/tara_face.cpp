#include "tara_face.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

TaraFace::TaraFace(IDisplay* display, int screenW, int screenH, uint8_t fps)
    : _d(display), _sw(screenW), _sh(screenH) {
    setFramerate(fps);
    _resetToDefaults();
}

// Sets every "current" and "target" value to the defaults so eyes start centred.
void TaraFace::_resetToDefaults() {
    eyeW  = eyeWDefault;  eyeH  = eyeHDefault;  eyeR  = eyeRDefault;
    eyeWt = eyeWDefault;  eyeHt = eyeHDefault;
    space = spaceDef;

    float totalW = eyeWDefault * 2 + spaceDef;
    leftX  = (_sw - totalW) / 2.0f;
    leftY  = (_sh - eyeHDefault) / 2.0f;
    leftXt = leftX;  leftYt = leftY;
    rightX = leftX + eyeWDefault + spaceDef;
    rightY = leftY;

    // Eyelid heights all start at zero (no mood)
    lidTired = lidAngry = lidHappy = 0;
    lidTiredT = lidAngryT = lidHappyT = 0;

    // Sweat drop positions — above the left eye
    sweat1y = leftY - 4;  sweat2y = leftY - 5;  sweat3y = leftY - 4;
    sweat1h = sweat2h = sweat3h = 1;
}

// ─── begin ────────────────────────────────────────────────────────────────────

void TaraFace::begin() {
    // Schedule the first auto-blink and idle move
    blinkTimer = millis() + _randomMs(blinkIntervalSec, blinkVarSec);
    idleTimer  = millis() + _randomMs(idleIntervalSec,  idleVarSec);

    // Map face states to moods/positions then call update()
    FaceRenderer r = {};
    r.render[FACE_BOOTING]        = nullptr;
    r.render[FACE_CONNECTING]     = [this]() { setPosition(TARA_CENTER);    update(); };
    r.render[FACE_REGISTERING]    = [this]() { setMood(TARA_DEFAULT);       update(); };
    r.render[FACE_WAITING_CONFIG] = [this]() { setMood(TARA_TIRED);         update(); };
    r.render[FACE_CONFIGURING]    = [this]() { setMood(TARA_DEFAULT);       update(); };
    r.render[FACE_IDLE]           = [this]() { setMood(TARA_DEFAULT);       update(); };
    r.render[FACE_LISTENING]      = [this]() { setMood(TARA_HAPPY);         update(); };
    r.render[FACE_THINKING]       = [this]() { setPosition(TARA_NE);        update(); };
    r.render[FACE_SPEAKING]       = [this]() { anim_laugh();                update(); };
    r.render[FACE_SLEEPING]       = [this]() { setMood(TARA_TIRED); close(); update(); };
    r.render[FACE_ERROR]          = [this]() { setMood(TARA_ANGRY); anim_confused(); update(); };
    face_register(r);
}

// ─── Eye shape ────────────────────────────────────────────────────────────────

void TaraFace::setWidth(uint8_t left, uint8_t right)        { eyeWDefault = left; eyeWt = left; }
void TaraFace::setHeight(uint8_t left, uint8_t right)       { eyeHDefault = left; eyeHt = left; }
void TaraFace::setBorderRadius(uint8_t left, uint8_t right) { eyeRDefault = left; eyeR  = left; }
void TaraFace::setSpaceBetween(int s)                       { spaceDef = s; space = s; }

// ─── Mood ─────────────────────────────────────────────────────────────────────
// Each mood sets the TARGET eyelid height. drawEyes() smoothly tweens toward it.
//
//  DEFAULT → all eyelids retract (targets = 0)
//  TIRED   → drooping top eyelid (triangle from top-left corner)
//  ANGRY   → sharp angled eyelid (triangle from top-right corner)
//  HAPPY   → bottom eyelid rises up (rounded rect from bottom)

void TaraFace::setMood(uint8_t mood) {
    _mood = mood;
    float maxLid = eyeHDefault / 2.0f;
    lidTiredT = lidAngryT = lidHappyT = 0;   // reset all first

    if (mood == TARA_TIRED)  lidTiredT = maxLid;
    if (mood == TARA_ANGRY)  lidAngryT = maxLid;
    if (mood == TARA_HAPPY)  lidHappyT = maxLid;
}

// ─── Position ─────────────────────────────────────────────────────────────────
// Sets the TARGET position for the left eye. The right eye always follows it.
// drawEyes() smoothly slides toward the target.

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
        default:      leftXt = cx;    leftYt = cy;   break;  // CENTER
    }
}

// ─── Curiosity / cyclops ──────────────────────────────────────────────────────

void TaraFace::setCuriosity(bool on) { curious = on; }

void TaraFace::setCyclops(bool on) {
    cyclops = on;
    // Right eye width/height target collapses to 0 in cyclops, restores otherwise
    eyeWt = on ? 0 : eyeWDefault;
    eyeHt = on ? 0 : eyeHDefault;
}

// ─── Blink ────────────────────────────────────────────────────────────────────
// close() sets the eye height TARGET to 1 px (nearly invisible).
// open()  restores the target to the default height.
// drawEyes() tweens smoothly — so the eye animates closed/open each frame.

void TaraFace::close(bool left, bool right) {
    if (left)  eyeHt = 1;
    if (right) eyeHt = 1;  // both eyes share one height target in this impl
}

void TaraFace::open(bool left, bool right) {
    if (left || right) eyeHt = eyeHDefault;
}

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

void TaraFace::setHFlicker(bool on, uint8_t amplitude) { hFlicker = on; hFlickerAmp = amplitude; }
void TaraFace::setVFlicker(bool on, uint8_t amplitude) { vFlicker = on; vFlickerAmp = amplitude; }
void TaraFace::setSweat(bool on) { sweat = on; }

// ─── One-shot animations ──────────────────────────────────────────────────────

void TaraFace::anim_confused() { if (!confused) { confused = true; confusedAt = millis(); } }
void TaraFace::anim_laugh()    { if (!laughing)  { laughing  = true; laughAt    = millis(); } }

// ─── Frame rate / constraints ─────────────────────────────────────────────────

void TaraFace::setFramerate(uint8_t fps)   { frameMs = fps > 0 ? 1000 / fps : 20; }
int  TaraFace::getScreenConstraintX()      { return _sw - eyeWDefault * 2 - spaceDef; }
int  TaraFace::getScreenConstraintY()      { return _sh - eyeHDefault; }

// ─── Helpers ──────────────────────────────────────────────────────────────────

// Tween: move halfway from current toward target each frame → smooth animation.
float TaraFace::_tween(float current, float target) { return (current + target) / 2.0f; }

// Random millisecond interval: base seconds + random 0..variation seconds
unsigned long TaraFace::_randomMs(int base, int variation) {
    return (unsigned long)(base + random(0, variation + 1)) * 1000UL;
}

// ─── Sweat drops ──────────────────────────────────────────────────────────────

void TaraFace::_tickSweat() {
    // Drops fall downward and grow, then reset to top when they go too low
    sweat1y += 0.5f;  sweat1h = constrain(sweat1h + 0.3f, 1.0f, 4.0f);
    sweat2y += 0.5f;  sweat2h = constrain(sweat2h + 0.3f, 1.0f, 4.0f);
    sweat3y += 0.5f;  sweat3h = constrain(sweat3h + 0.3f, 1.0f, 4.0f);

    if (sweat1y > leftY + eyeH + 4) {   // reset all when first drop exits
        sweat1y = leftY - 4;  sweat1h = 1;
        sweat2y = leftY - 5;  sweat2h = 1;
        sweat3y = leftY - 4;  sweat3h = 1;
    }

    float sx = leftX;
    _d->fillRoundRect((int)sx,                    (int)sweat1y, 2, (int)sweat1h, 1, true);
    _d->fillRoundRect((int)(sx + eyeWDefault/2),  (int)sweat2y, 2, (int)sweat2h, 1, true);
    _d->fillRoundRect((int)(sx + eyeWDefault - 2),(int)sweat3y, 2, (int)sweat3h, 1, true);
}

// ─── update / drawEyes ────────────────────────────────────────────────────────

// update() — call in loop(). Skips drawing if not enough time has passed.
void TaraFace::update() {
    if (millis() - lastFrameMs < (unsigned long)frameMs) return;
    lastFrameMs = millis();
    drawEyes();
}

// drawEyes() — the main render function, split into five clear steps:
//   1. Tick one-shot animations (confused shake, laugh bounce)
//   2. Tick automated events (auto-blink, idle reposition)
//   3. Calculate flicker offsets for this frame
//   4. Smooth-move all geometry toward their targets (tweening)
//   5. Draw: clear screen → left eye → right eye → eyelids → sweat

void TaraFace::drawEyes() {
    unsigned long now = millis();

    // ── Step 1: one-shot animations ───────────────────────────────────────────
    if (confused) {
        hFlicker = true;  hFlickerAmp = 20;
        if (now - confusedAt >= CONFUSED_DUR) { confused = false; hFlicker = false; }
    }
    if (laughing) {
        vFlicker = true;  vFlickerAmp = 5;
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

    // ── Step 3: flicker offsets (alternates ±amplitude each frame) ────────────
    int hOff = 0, vOff = 0;
    if (hFlicker) { hFlickerAlt = !hFlickerAlt; hOff = hFlickerAlt ? hFlickerAmp : -hFlickerAmp; }
    if (vFlicker) { vFlickerAlt = !vFlickerAlt; vOff = vFlickerAlt ? vFlickerAmp : -vFlickerAmp; }

    // ── Step 4: tween all geometry toward their targets ───────────────────────
    eyeW = _tween(eyeW, eyeWt);
    eyeH = _tween(eyeH, eyeHt);

    leftX  = _tween(leftX,  leftXt);
    leftY  = _tween(leftY,  leftYt);
    rightX = leftX + eyeW + space;
    rightY = leftY;

    // Curiosity: the outer eye grows taller when looking far left or right
    float lExtra = 0, rExtra = 0;
    if (curious) {
        if (leftX  <= 10)                     lExtra = 8;
        if (!cyclops && rightX >= _sw - eyeW - 10) rExtra = 8;
    }

    // Tween eyelids toward their targets
    lidTired = _tween(lidTired, lidTiredT);
    lidAngry = _tween(lidAngry, lidAngryT);
    lidHappy = _tween(lidHappy, lidHappyT);

    // ── Step 5: draw ──────────────────────────────────────────────────────────
    _d->clear();
    _d->fillScreen(false);   // black background

    // Left eye
    int lx = (int)leftX  + hOff,  ly = (int)(leftY  - lExtra / 2) + vOff;
    int lw = (int)eyeW,            lh = (int)(eyeH + lExtra);

    if (lw > 0 && lh > 0) {
        _d->fillRoundRect(lx, ly, lw, lh, (int)eyeR, true);
        _drawEyelids(lx, ly, lw, lh, true);
    }

    // Right eye (skipped in cyclops mode)
    int rx = (int)rightX + hOff,  ry = (int)(rightY - rExtra / 2) + vOff;
    int rw = cyclops ? 0 : (int)eyeW,  rh = cyclops ? 0 : (int)(eyeH + rExtra);

    if (rw > 0 && rh > 0) {
        _d->fillRoundRect(rx, ry, rw, rh, (int)eyeR, true);
        _drawEyelids(rx, ry, rw, rh, false);
    }

    if (sweat) _tickSweat();

    _d->show();
}

// Draw eyelid overlays on top of one eye.
// Each mood masks part of the eye with a black shape:
//   TIRED  — black triangle from top-left  (drooping inner corner)
//   ANGRY  — black triangle from top-right (sharp outer corner)
//   HAPPY  — black rounded rect from bottom (cheek rises up)
void TaraFace::_drawEyelids(int x, int y, int w, int h, bool isLeft) {
    if (lidTired > 0) {
        int lh = (int)lidTired;
        if (isLeft)
            _d->fillTriangle(x,     y - 1, x + w, y - 1, x,     y + lh - 1, false);
        else
            _d->fillTriangle(x,     y - 1, x + w, y - 1, x + w, y + lh - 1, false);
    }
    if (lidAngry > 0) {
        int lh = (int)lidAngry;
        if (isLeft)
            _d->fillTriangle(x,     y - 1, x + w, y - 1, x + w, y + lh - 1, false);
        else
            _d->fillTriangle(x,     y - 1, x + w, y - 1, x,     y + lh - 1, false);
    }
    if (lidHappy > 0) {
        int offset = (int)lidHappy;
        _d->fillRoundRect(x - 1, (y + h) - offset + 1, w + 2, eyeHDefault, (int)eyeR, false);
    }
}
