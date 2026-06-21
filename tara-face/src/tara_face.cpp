#include "tara_face.h"
#include "idle_face.h"

TaraFace::TaraFace(IDisplay* display, int screenW, int screenH)
    : _d(display), _sw(screenW), _sh(screenH) {}

void TaraFace::begin() {
    FaceRenderer r = {};
    r.render[FACE_IDLE] = [this]() { drawIdle(); };
    face_register(r);
}

void TaraFace::drawIdle() {
    renderIdleFace(_d, _sw, _sh, _eyesOpen, _lastBlink);
}
