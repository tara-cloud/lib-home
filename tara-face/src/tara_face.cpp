#include "tara_face.h"

TaraFace::TaraFace(IDisplay* display) : _display(display), _fallback(nullptr) {
    for (int i = 0; i < FACE_STATE_COUNT; i++)
        _handlers[i] = nullptr;
}

TaraFace& TaraFace::on(FaceState state, TaraFaceRenderFn fn) {
    if (state >= 0 && state < FACE_STATE_COUNT)
        _handlers[state] = fn;
    return *this;
}

TaraFace& TaraFace::setFallback(TaraFaceRenderFn fn) {
    _fallback = fn;
    return *this;
}

void TaraFace::begin() {
    FaceRenderer r = {};
    for (int i = 0; i < FACE_STATE_COUNT; i++) {
        if (_handlers[i])
            r.render[i] = _handlers[i];
        else if (_fallback)
            r.render[i] = _fallback;
    }
    face_register(r);
}

IDisplay* TaraFace::display() const {
    return _display;
}
