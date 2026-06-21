#include "idle_face.h"
#include <Arduino.h>

// Eye geometry (preferred, user-confirmed: 36×36 r=8 space=10)
static const int _EYE_W = 36;
static const int _EYE_H = 36;
static const int _EYE_R = 8;
static const int _SPACE = 10;

// Draw both eyes with a lid covering the top lidH pixels (0 = fully open).
// When lidH >= EYE_H the eye is fully closed — show a thin bottom line instead.
static void _drawEyes(IDisplay* d, int leftX, int rightX, int eyeY, int lidH) {
    d->clear();
    d->fillScreen(false);   // black background

    for (int ex : {leftX, rightX}) {
        if (lidH >= _EYE_H) {
            // Fully closed — thin line at the bottom of the eye
            d->fillRect(ex, eyeY + _EYE_H - 3, _EYE_W, 3, true);
        } else {
            // White eye body
            d->fillRoundRect(ex, eyeY, _EYE_W, _EYE_H, _EYE_R, true);
            // Black lid descending from the top
            if (lidH > 0)
                d->fillRect(ex, eyeY, _EYE_W, lidH, false);
        }
    }

    d->show();
}

void renderIdleFace(IDisplay* display, int screenW, int screenH,
                    BlinkState& blink) {
    unsigned long now    = millis();
    unsigned long elapsed = now - blink.phaseAt;

    // ── Advance state machine ─────────────────────────────────────────────────
    switch (blink.phase) {
        case BLINK_WAITING:
            if (elapsed >= BLINK_WAIT_MS) {
                blink.phase   = BLINK_CLOSING;
                blink.phaseAt = now;
                elapsed       = 0;
            }
            break;
        case BLINK_CLOSING:
            if (elapsed >= BLINK_CLOSE_MS) {
                blink.phase   = BLINK_OPENING;
                blink.phaseAt = now;
                elapsed       = 0;
            }
            break;
        case BLINK_OPENING:
            if (elapsed >= BLINK_OPEN_MS) {
                blink.phase   = BLINK_WAITING;
                blink.phaseAt = now;
                elapsed       = 0;
            }
            break;
    }

    // ── Calculate lid height for this frame ───────────────────────────────────
    int lidH = 0;
    if (blink.phase == BLINK_CLOSING) {
        // 0 → EYE_H  over BLINK_CLOSE_MS
        float t = (float)elapsed / BLINK_CLOSE_MS;
        lidH = (int)(t * (_EYE_H + 1));
    } else if (blink.phase == BLINK_OPENING) {
        // EYE_H → 0  over BLINK_OPEN_MS
        float t = (float)elapsed / BLINK_OPEN_MS;
        lidH = (int)((1.0f - t) * (_EYE_H + 1));
    }
    lidH = constrain(lidH, 0, _EYE_H);

    // ── Layout ────────────────────────────────────────────────────────────────
    int totalW = _EYE_W + _SPACE + _EYE_W;
    int leftX  = (screenW - totalW) / 2;
    int rightX = leftX + _EYE_W + _SPACE;
    int eyeY   = (screenH - _EYE_H) / 2;

    _drawEyes(display, leftX, rightX, eyeY, lidH);
}
