#include "giggle_face.h"
#include <Arduino.h>

// Eye geometry (same as idle — 36×36 r=8 space=10)
static const int _GG_EYE_W = 36;
static const int _GG_EYE_H = 10;   // squinted height
static const int _GG_EYE_R = 4;
static const int _GG_SPACE  = 10;

static void _drawGiggleEyes(IDisplay* d, int screenW, int screenH, int offsetY) {
    int totalW = _GG_EYE_W + _GG_SPACE + _GG_EYE_W;
    int leftX  = (screenW - totalW) / 2;
    int rightX = leftX + _GG_EYE_W + _GG_SPACE;
    int eyeY   = (screenH - _GG_EYE_H) / 2 + offsetY;

    d->clear();
    d->fillScreen(false);
    d->fillRoundRect(leftX,  eyeY, _GG_EYE_W, _GG_EYE_H, _GG_EYE_R, true);
    d->fillRoundRect(rightX, eyeY, _GG_EYE_W, _GG_EYE_H, _GG_EYE_R, true);
    d->show();
}

bool renderGiggleFace(IDisplay* display, int screenW, int screenH,
                      GiggleState& state) {
    unsigned long now = millis();

    if (!state.active) {
        state.active  = true;
        state.startAt = now;
        state.bounce  = 0;
    }

    unsigned long elapsed = now - state.startAt;
    int cycle = (int)(elapsed / GIGGLE_BOUNCE_MS);

    if (cycle >= GIGGLE_BOUNCES * 2) {
        // Animation done — draw neutral and reset
        _drawGiggleEyes(display, screenW, screenH, 0);
        state.active = false;
        return false;
    }

    // Bounce: alternate up (-4px) and down (+4px)
    int offsetY = (cycle % 2 == 0) ? -4 : 4;
    _drawGiggleEyes(display, screenW, screenH, offsetY);
    return true;
}
