#include "face.h"

static FaceRenderer _renderer;
static bool         _registered = false;

void face_register(const FaceRenderer& renderer) {
    _renderer   = renderer;
    _registered = true;
}

void renderFace(FaceState state) {
    if (!_registered) return;
    if (state < 0 || state >= FACE_STATE_COUNT) return;
    if (_renderer.render[state]) _renderer.render[state]();
}
