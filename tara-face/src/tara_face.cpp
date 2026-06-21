#include "tara_face.h"

TaraFace::TaraFace(IDisplay* display, int screenW, int screenH)
    : _d(display), _sw(screenW), _sh(screenH) {}

void TaraFace::begin() {
    FaceRenderer r = {};
    r.render[FACE_IDLE] = [this]() { drawIdle(); };
    face_register(r);
}

// Toggle open/closed every 250 ms
void TaraFace::drawIdle() {
    unsigned long now = millis();
    if (now - _lastBlink >= BLINK_INTERVAL) {
        _eyesOpen  = !_eyesOpen;
        _lastBlink = now;
    }

    int totalW = EYE_W + SPACE + EYE_W;
    int leftX  = (_sw - totalW) / 2;
    int rightX = leftX + EYE_W + SPACE;
    int eyeY   = (_sh - EYE_H) / 2;

    _d->clear();
    _d->fillScreen(false);   // black background

    if (_eyesOpen)
        _drawOpen(leftX, rightX, eyeY);
    else
        _drawClosed(leftX, rightX, eyeY);

    _d->show();
}

// Open: full rounded-rect eye
void TaraFace::_drawOpen(int leftX, int rightX, int eyeY) {
    _d->fillRoundRect(leftX,  eyeY, EYE_W, EYE_H, EYE_R, true);
    _d->fillRoundRect(rightX, eyeY, EYE_W, EYE_H, EYE_R, true);
}

// Closed: thin horizontal line in the middle of where each eye was
void TaraFace::_drawClosed(int leftX, int rightX, int eyeY) {
    int midY = eyeY + EYE_H / 2 - 1;
    _d->fillRect(leftX,  midY, EYE_W, 3, true);
    _d->fillRect(rightX, midY, EYE_W, 3, true);
}
