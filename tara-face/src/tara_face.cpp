#include "tara_face.h"

TaraFace::TaraFace(IDisplay* display, int screenW, int screenH)
    : _d(display), _sw(screenW), _sh(screenH) {}

void TaraFace::begin() {
    FaceRenderer r = {};
    r.render[FACE_IDLE] = [this]() { drawIdle(); };
    face_register(r);
}

void TaraFace::drawIdle() {
    // Centre both eyes on screen
    int totalW = EYE_W + SPACE + EYE_W;
    int leftX  = (_sw - totalW) / 2;
    int rightX = leftX + EYE_W + SPACE;
    int eyeY   = (_sh - EYE_H) / 2;

    _d->clear();
    _d->fillScreen(false);                              // black background
    _d->fillRoundRect(leftX,  eyeY, EYE_W, EYE_H, EYE_R, true);   // left eye
    _d->fillRoundRect(rightX, eyeY, EYE_W, EYE_H, EYE_R, true);   // right eye
    _d->show();
}
