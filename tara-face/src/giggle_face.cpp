#include "giggle_face.h"
#include <Arduino.h>

static const int _GG_EYE_W = 36;
static const int _GG_EYE_H = 28;   // tall = thick visible arch
static const int _GG_EYE_R = 14;   // large radius = smooth round top
static const int _GG_SPACE  = 10;
static const int _GG_MASK   = 10;  // only mask this many px from bottom → thick arch

// Draw one arch eye at (x, y).
// Arch = filled rounded rect with bottom half masked black → only top curve visible.
static void _drawArch(IDisplay* d, int x, int y) {
    // Full rounded rect (white)
    d->fillRoundRect(x, y, _GG_EYE_W, _GG_EYE_H, _GG_EYE_R, true);
    // Mask only the bottom strip → leaves a thick happy arch
    d->fillRect(x, y + _GG_EYE_H - _GG_MASK, _GG_EYE_W, _GG_MASK + 1, false);
}

static void _drawGiggleEyes(IDisplay* d, int screenW, int screenH,
                             int offsetX, int offsetY) {
    int totalW = _GG_EYE_W + _GG_SPACE + _GG_EYE_W;
    int baseX  = (screenW - totalW) / 2;
    int eyeY   = (screenH - _GG_EYE_H / 2) / 2 + offsetY;  // centre the visible arch

    int leftX  = baseX + offsetX;
    int rightX = baseX + _GG_EYE_W + _GG_SPACE + offsetX;

    d->clear();
    d->fillScreen(false);
    _drawArch(d, leftX,  eyeY);
    _drawArch(d, rightX, eyeY);
    d->show();
}

bool renderGiggleFace(IDisplay* display, int screenW, int screenH,
                      GiggleState& state) {
    unsigned long now = millis();

    if (!state.active) {
        state.active  = true;
        state.startAt = now;
    }

    unsigned long elapsed = now - state.startAt;
    int cycle = (int)(elapsed / GIGGLE_BOUNCE_MS);

    if (cycle >= GIGGLE_BOUNCES * 2) {
        _drawGiggleEyes(display, screenW, screenH, 0, 0);
        state.active = false;
        return false;
    }

    // Shake: alternate left (-5px) and right (+5px)
    int offsetX = (cycle % 2 == 0) ? -5 : 5;
    // Slight vertical bob too
    int offsetY = (cycle % 4 < 2) ? -2 : 2;

    _drawGiggleEyes(display, screenW, screenH, offsetX, offsetY);
    return true;
}
