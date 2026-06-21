#include "tara_face.h"
#include "idle_face.h"
#include "giggle_face.h"

TaraFace::TaraFace(IDisplay* display, int screenW, int screenH)
    : _d(display), _sw(screenW), _sh(screenH) {}

void TaraFace::begin() {
    FaceRenderer r = {};
    r.render[FACE_IDLE]   = [this]() { drawIdle(); };
    r.render[FACE_GIGGLE] = [this]() { drawGiggle(); };
    face_register(r);
}

void TaraFace::drawIdle() {
    renderIdleFace(_d, _sw, _sh, _blink);
}

void TaraFace::drawGiggle() {
    // When animation finishes, revert to idle
    if (!renderGiggleFace(_d, _sw, _sh, _giggle)) {
        _blink = {};   // reset blink state so idle starts fresh
    }
}
