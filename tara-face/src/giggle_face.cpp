#include "giggle_face.h"
#include <Arduino.h>

// Eye shape from the image: small pill/capsule eyes, close together, upper-centre.
// Width narrow, height moderate, very round (r = w/2 makes a true capsule).
// Slightly inward tilt simulated by masking the outer-top corner of each eye.
static const int _GG_EYE_W  = 22;   // narrow pill
static const int _GG_EYE_H  = 14;   // taller than wide → capsule feel
static const int _GG_EYE_R  = 7;    // r = w/2 = full capsule ends
static const int _GG_SPACE  = 14;   // close together
static const int _GG_TILT   = 3;    // px — inner-top corner masked for inward lean

// Draw one capsule eye with inward tilt mask.
// isLeft=true masks top-right corner (leans right), isLeft=false masks top-left.
static void _drawCapsule(IDisplay* d, int x, int y, bool isLeft) {
    d->fillRoundRect(x, y, _GG_EYE_W, _GG_EYE_H, _GG_EYE_R, true);
    // Inward tilt — mask stepped triangle at inner-top corner
    for (int t = 0; t < _GG_TILT; t++) {
        int maskW = _GG_TILT - t;
        if (isLeft)
            d->fillRect(x + _GG_EYE_W - maskW, y + t, maskW, 1, false);  // right side of left eye
        else
            d->fillRect(x, y + t, maskW, 1, false);                        // left side of right eye
    }
}

static void _drawGiggleFrame(IDisplay* d, int screenW, int screenH,
                              int offsetX, int scaleH) {
    int totalW = _GG_EYE_W + _GG_SPACE + _GG_EYE_W;
    int leftX  = (screenW - totalW) / 2 + offsetX;
    int rightX = leftX + _GG_EYE_W + _GG_SPACE;

    // Position eyes in upper third of screen (like the image)
    int eyeY = screenH / 4;

    // Pulse: temporarily taller eyes on even cycle
    int h = _GG_EYE_H + scaleH;
    int r = h / 2;

    d->clear();
    d->fillScreen(false);

    // Left eye
    d->fillRoundRect(leftX, eyeY, _GG_EYE_W, h, r, true);
    for (int t = 0; t < _GG_TILT; t++) {
        int mw = _GG_TILT - t;
        d->fillRect(leftX + _GG_EYE_W - mw, eyeY + t, mw, 1, false);
    }

    // Right eye
    d->fillRoundRect(rightX, eyeY, _GG_EYE_W, h, r, true);
    for (int t = 0; t < _GG_TILT; t++) {
        int mw = _GG_TILT - t;
        d->fillRect(rightX, eyeY + t, mw, 1, false);
    }

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
        // End — draw clean capsule eyes centred, no offset
        _drawGiggleFrame(display, screenW, screenH, 0, 0);
        state.active = false;
        return false;
    }

    // Tiny left/right wobble (±3px) + pulse height (+4px on every other cycle)
    int offsetX = (cycle % 2 == 0) ? -3 : 3;
    int scaleH  = (cycle % 2 == 0) ?  4 : 0;

    _drawGiggleFrame(display, screenW, screenH, offsetX, scaleH);
    return true;
}
