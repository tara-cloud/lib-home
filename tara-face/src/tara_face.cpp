#include "tara_face.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

TaraFace::TaraFace(IDisplay* display, int screenW, int screenH, uint8_t fps)
    : _d(display), _sw(screenW), _sh(screenH) {
    setFramerate(fps);
    _initPositions();
}

void TaraFace::_initPositions() {
    // Current geometry = defaults
    _eyeLwC = _eyeLwD; _eyeLhC = _eyeLhD; _eyeLrC = _eyeLrD;
    _eyeRwC = _eyeRwD; _eyeRhC = _eyeRhD; _eyeRrC = _eyeRrD;
    _spaceC = _spaceBetweenD;
    _eyeLwN = _eyeLwD; _eyeLhN = _eyeLhD;
    _eyeRwN = _eyeRwD; _eyeRhN = _eyeRhD;

    // Centre eyes on screen
    float totalW = _eyeLwD + _spaceBetweenD + _eyeRwD;
    _eyeLx = (_sw - totalW) / 2.0f;
    _eyeLy = (_sh - _eyeLhD) / 2.0f;
    _eyeRx = _eyeLx + _eyeLwD + _spaceBetweenD;
    _eyeRy = _eyeLy;
    _eyeLxN = _eyeLx; _eyeLyN = _eyeLy;

    // Sweat positions (above left eye)
    _sweat1x = _eyeLx;                          _sweat1y = _eyeLy - 4;
    _sweat2x = _eyeLx + _eyeLwD / 2 - 1;        _sweat2y = _eyeLy - 5;
    _sweat3x = _eyeLx + _eyeLwD - 2;             _sweat3y = _eyeLy - 4;
}

// ─── begin ────────────────────────────────────────────────────────────────────

void TaraFace::begin() {
    // Auto-blink and idle timers
    _blinkTimer = millis() + (unsigned long)(_blinkIntervalSec + random(0, _blinkVarSec + 1)) * 1000UL;
    _idleTimer  = millis() + (unsigned long)(_idleIntervalSec  + random(0, _idleVarSec  + 1)) * 1000UL;

    // Register with face.h dispatcher — idle maps to update(), others map to
    // single drawEyes() after optionally setting mood/position
    FaceRenderer r = {};
    r.render[FACE_BOOTING]        = nullptr;
    r.render[FACE_CONNECTING]     = [this]() { setPosition(TARA_CENTER); update(); };
    r.render[FACE_REGISTERING]    = [this]() { setMood(TARA_DEFAULT); update(); };
    r.render[FACE_WAITING_CONFIG] = [this]() { setMood(TARA_TIRED); update(); };
    r.render[FACE_CONFIGURING]    = [this]() { setMood(TARA_DEFAULT); update(); };
    r.render[FACE_IDLE]           = [this]() { setMood(TARA_DEFAULT); update(); };
    r.render[FACE_LISTENING]      = [this]() { setMood(TARA_HAPPY); update(); };
    r.render[FACE_THINKING]       = [this]() { setPosition(TARA_NE); update(); };
    r.render[FACE_SPEAKING]       = [this]() { anim_laugh(); update(); };
    r.render[FACE_SLEEPING]       = [this]() { setMood(TARA_TIRED); close(); update(); };
    r.render[FACE_ERROR]          = [this]() { setMood(TARA_ANGRY); anim_confused(); update(); };
    face_register(r);
}

// ─── Eye shape setters ────────────────────────────────────────────────────────

void TaraFace::setWidth(uint8_t left, uint8_t right) {
    _eyeLwD = left; _eyeRwD = right;
    _eyeLwN = left; _eyeRwN = right;
}

void TaraFace::setHeight(uint8_t left, uint8_t right) {
    _eyeLhD = left; _eyeRhD = right;
    _eyeLhN = left; _eyeRhN = right;
}

void TaraFace::setBorderRadius(uint8_t left, uint8_t right) {
    _eyeLrD = left; _eyeRrD = right;
    _eyeLrC = left; _eyeRrC = right;
}

void TaraFace::setSpaceBetween(int space) {
    _spaceBetweenD = space;
    _spaceC = space;
}

// ─── Mood ─────────────────────────────────────────────────────────────────────

void TaraFace::setMood(uint8_t mood) {
    _mood = mood;
    _applyMoodTargets();
}

void TaraFace::_applyMoodTargets() {
    float maxLid = _eyeLhD / 2.0f;
    // Reset all lid targets
    _lidTiredLN = _lidTiredRN = 0;
    _lidAngryLN = _lidAngryRN = 0;
    _lidHappyLN = _lidHappyRN = 0;

    switch (_mood) {
        case TARA_TIRED:
            _lidTiredLN = maxLid;
            _lidTiredRN = _cyclops ? 0 : maxLid;
            break;
        case TARA_ANGRY:
            _lidAngryLN = maxLid;
            _lidAngryRN = _cyclops ? 0 : maxLid;
            break;
        case TARA_HAPPY:
            _lidHappyLN = maxLid;
            _lidHappyRN = _cyclops ? 0 : maxLid;
            break;
        default: break;
    }
}

// ─── Position ─────────────────────────────────────────────────────────────────

void TaraFace::setPosition(uint8_t pos) {
    int maxX = getScreenConstraintX();
    int maxY = getScreenConstraintY();
    float cx = (_sw - (_eyeLwD + _spaceBetweenD + _eyeRwD)) / 2.0f;
    float cy = (_sh - _eyeLhD) / 2.0f;

    switch (pos) {
        case TARA_N:   _eyeLxN = cx;      _eyeLyN = 0;    break;
        case TARA_NE:  _eyeLxN = maxX;    _eyeLyN = 0;    break;
        case TARA_E:   _eyeLxN = maxX;    _eyeLyN = cy;   break;
        case TARA_SE:  _eyeLxN = maxX;    _eyeLyN = maxY; break;
        case TARA_S:   _eyeLxN = cx;      _eyeLyN = maxY; break;
        case TARA_SW:  _eyeLxN = 0;       _eyeLyN = maxY; break;
        case TARA_W:   _eyeLxN = 0;       _eyeLyN = cy;   break;
        case TARA_NW:  _eyeLxN = 0;       _eyeLyN = 0;    break;
        default:       _eyeLxN = cx;      _eyeLyN = cy;   break; // CENTER
    }
}

// ─── Curiosity / cyclops ──────────────────────────────────────────────────────

void TaraFace::setCuriosity(bool on) { _curious = on; }

void TaraFace::setCyclops(bool on) {
    _cyclops = on;
    if (on) {
        _eyeRwN = 0; _eyeRhN = 0; _spaceC = 0;
    } else {
        _eyeRwN = _eyeRwD; _eyeRhN = _eyeRhD; _spaceC = _spaceBetweenD;
    }
    _applyMoodTargets();
}

// ─── Blink control ────────────────────────────────────────────────────────────

void TaraFace::close(bool left, bool right) {
    if (left)  { _eyeLhN = 1; _eyeLclosed = true; }
    if (right) { _eyeRhN = 1; _eyeRclosed = true; }
}

void TaraFace::open(bool left, bool right) {
    if (left)  { _eyeLhN = _eyeLhD; _eyeLclosed = false; }
    if (right) { _eyeRhN = _eyeRhD; _eyeRclosed = false; }
}

void TaraFace::blink(bool left, bool right) { close(left, right); open(left, right); }

void TaraFace::close() { close(true, true); }
void TaraFace::open()  { open(true, true); }
void TaraFace::blink() { blink(true, true); }

// ─── Automated animations ────────────────────────────────────────────────────

void TaraFace::setAutoblinker(bool on, int intervalSec, int variationSec) {
    _autoBlink = on;
    _blinkIntervalSec = intervalSec;
    _blinkVarSec = variationSec;
    if (on) _blinkTimer = millis() + (unsigned long)(intervalSec + random(0, variationSec + 1)) * 1000UL;
}

void TaraFace::setIdleMode(bool on, int intervalSec, int variationSec) {
    _idle = on;
    _idleIntervalSec = intervalSec;
    _idleVarSec = variationSec;
    if (on) _idleTimer = millis() + (unsigned long)(intervalSec + random(0, variationSec + 1)) * 1000UL;
}

// ─── Effects ──────────────────────────────────────────────────────────────────

void TaraFace::setHFlicker(bool on, uint8_t amplitude) { _hFlicker = on; _hFlickerAmp = amplitude; }
void TaraFace::setVFlicker(bool on, uint8_t amplitude) { _vFlicker = on; _vFlickerAmp = amplitude; }
void TaraFace::setSweat(bool on) { _sweat = on; }

// ─── One-shot animations ──────────────────────────────────────────────────────

void TaraFace::anim_confused() {
    if (!_confused) { _confused = true; _confusedStart = millis(); }
}

void TaraFace::anim_laugh() {
    if (!_laughing) { _laughing = true; _laughStart = millis(); }
}

// ─── Frame rate ───────────────────────────────────────────────────────────────

void TaraFace::setFramerate(uint8_t fps) {
    _frameInterval = fps > 0 ? 1000 / fps : 20;
}

// ─── Constraint helpers ───────────────────────────────────────────────────────

int TaraFace::getScreenConstraintX() {
    return _sw - _eyeLwD - _spaceBetweenD - _eyeRwD;
}

int TaraFace::getScreenConstraintY() {
    return _sh - _eyeLhD;
}

// ─── Tween helper ─────────────────────────────────────────────────────────────

float TaraFace::_tween(float cur, float target) {
    return (cur + target) / 2.0f;
}

// ─── Sweat drawing ────────────────────────────────────────────────────────────

void TaraFace::_drawSweat() {
    // Move drops down, grow then shrink
    _sweat1y += 0.5f; _sweat2y += 0.5f; _sweat3y += 0.5f;
    _sweat1h = constrain(_sweat1h + 0.3f, 1, 4);
    _sweat2h = constrain(_sweat2h + 0.3f, 1, 4);
    _sweat3h = constrain(_sweat3h + 0.3f, 1, 4);

    if (_sweat1y > _eyeLy + _eyeLhC + 4) {
        _sweat1y = _eyeLy - 4; _sweat1h = 1;
        _sweat2y = _eyeLy - 5; _sweat2h = 1;
        _sweat3y = _eyeLy - 4; _sweat3h = 1;
    }

    _d->fillRoundRect((int)_sweat1x, (int)_sweat1y, 2, (int)_sweat1h, 1, true);
    _d->fillRoundRect((int)_sweat2x, (int)_sweat2y, 2, (int)_sweat2h, 1, true);
    _d->fillRoundRect((int)_sweat3x, (int)_sweat3y, 2, (int)_sweat3h, 1, true);
}

// ─── update / drawEyes ────────────────────────────────────────────────────────

void TaraFace::update() {
    if (millis() - _fpsTimer < (unsigned long)_frameInterval) return;
    _fpsTimer = millis();
    drawEyes();
}

void TaraFace::drawEyes() {
    unsigned long now = millis();

    // ── One-shot animations ───────────────────────────────────────────────────
    if (_confused) {
        setHFlicker(true, 20);
        if (now - _confusedStart >= _CONFUSED_DUR) {
            _confused = false;
            setHFlicker(false);
        }
    }
    if (_laughing) {
        setVFlicker(true, 5);
        if (now - _laughStart >= _LAUGH_DUR) {
            _laughing = false;
            setVFlicker(false);
        }
    }

    // ── Autoblinker ───────────────────────────────────────────────────────────
    if (_autoBlink && now >= _blinkTimer) {
        blink();
        _blinkTimer = now + (unsigned long)(_blinkIntervalSec + random(0, _blinkVarSec + 1)) * 1000UL;
    }

    // ── Idle repositioning ────────────────────────────────────────────────────
    if (_idle && now >= _idleTimer) {
        _eyeLxN = random(0, getScreenConstraintX() + 1);
        _eyeLyN = random(0, getScreenConstraintY() + 1);
        _idleTimer = now + (unsigned long)(_idleIntervalSec + random(0, _idleVarSec + 1)) * 1000UL;
    }

    // ── Flicker offsets ───────────────────────────────────────────────────────
    int hOff = 0, vOff = 0;
    if (_hFlicker) { _hFlickerAlt = !_hFlickerAlt; hOff = _hFlickerAlt ? _hFlickerAmp : -_hFlickerAmp; }
    if (_vFlicker) { _vFlickerAlt = !_vFlickerAlt; vOff = _vFlickerAlt ? _vFlickerAmp : -_vFlickerAmp; }

    // ── Tween geometry ────────────────────────────────────────────────────────
    _eyeLwC = _tween(_eyeLwC, _eyeLwN);
    _eyeLhC = _tween(_eyeLhC, _eyeLhN);
    _eyeRwC = _tween(_eyeRwC, _eyeRwN);
    _eyeRhC = _tween(_eyeRhC, _eyeRhN);
    _eyeLx  = _tween(_eyeLx,  _eyeLxN);
    _eyeLy  = _tween(_eyeLy,  _eyeLyN);
    _eyeRx  = _eyeLx + _eyeLwC + _tween(_spaceC, _spaceBetweenD);
    _eyeRy  = _eyeLy;

    // ── Curiosity: outer eye gets taller ──────────────────────────────────────
    _eyeLhOffset = 0; _eyeRhOffset = 0;
    if (_curious) {
        if (_eyeLx <= 10)                                       _eyeLhOffset = 8;
        if (!_cyclops && _eyeRx >= _sw - _eyeRwC - 10)         _eyeRhOffset = 8;
    }

    // ── Tween eyelids ─────────────────────────────────────────────────────────
    _lidTiredL = _tween(_lidTiredL, _lidTiredLN);
    _lidTiredR = _tween(_lidTiredR, _lidTiredRN);
    _lidAngryL = _tween(_lidAngryL, _lidAngryLN);
    _lidAngryR = _tween(_lidAngryR, _lidAngryRN);
    _lidHappyL = _tween(_lidHappyL, _lidHappyLN);
    _lidHappyR = _tween(_lidHappyR, _lidHappyRN);

    // ── Render ────────────────────────────────────────────────────────────────
    _d->clear();
    _d->fillScreen(false);   // black background

    int lx = (int)_eyeLx + hOff;
    int ly = (int)(_eyeLy - _eyeLhOffset / 2.0f) + vOff;
    int lw = (int)_eyeLwC;
    int lh = (int)(_eyeLhC + _eyeLhOffset);
    int lr = (int)_eyeLrC;

    int rx = (int)_eyeRx + hOff;
    int ry = (int)(_eyeRy - _eyeRhOffset / 2.0f) + vOff;
    int rw = (int)_eyeRwC;
    int rh = (int)(_eyeRhC + _eyeRhOffset);
    int rr = (int)_eyeRrC;

    // Left eye
    if (lw > 0 && lh > 0) {
        _d->fillRoundRect(lx, ly, lw, lh, lr, true);

        // Tired eyelid — top-left triangle
        if (_lidTiredL > 0) {
            int h = (int)_lidTiredL;
            _d->fillTriangle(lx, ly - 1, lx + lw, ly - 1, lx, ly + h - 1, false);
        }
        // Angry eyelid — top-right triangle (inner corner)
        if (_lidAngryL > 0) {
            int h = (int)_lidAngryL;
            _d->fillTriangle(lx, ly - 1, lx + lw, ly - 1, lx + lw, ly + h - 1, false);
        }
        // Happy eyelid — bottom rounded rect
        if (_lidHappyL > 0) {
            int offset = (int)_lidHappyL;
            _d->fillRoundRect(lx - 1, (ly + lh) - offset + 1, lw + 2, _eyeLhD, lr, false);
        }
    }

    // Right eye
    if (!_cyclops && rw > 0 && rh > 0) {
        _d->fillRoundRect(rx, ry, rw, rh, rr, true);

        if (_lidTiredR > 0) {
            int h = (int)_lidTiredR;
            _d->fillTriangle(rx, ry - 1, rx + rw, ry - 1, rx + rw, ry + h - 1, false);
        }
        if (_lidAngryR > 0) {
            int h = (int)_lidAngryR;
            _d->fillTriangle(rx, ry - 1, rx + rw, ry - 1, rx, ry + h - 1, false);
        }
        if (_lidHappyR > 0) {
            int offset = (int)_lidHappyR;
            _d->fillRoundRect(rx - 1, (ry + rh) - offset + 1, rw + 2, _eyeRhD, rr, false);
        }
    }

    if (_sweat) _drawSweat();

    _d->show();
}
