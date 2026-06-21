#include "idle_face.h"
#include <Arduino.h>

// Eye geometry (preferred, user-confirmed: 36×36 r=8 space=10)
static const int _EYE_W = 36;
static const int _EYE_H = 36;
static const int _EYE_R = 8;
static const int _SPACE = 10;

// Draw both eyes with a lid covering the top lidH pixels (0 = fully open).
// Shadow: a 2 px white copy offset -2 px left and +2 px down, drawn first.
// The eye body draws on top — shadow peeks out on left and bottom edges only.
static void _drawEyes(IDisplay* d, int leftX, int rightX, int eyeY, int lidH) {
    d->clear();
    d->fillScreen(false);   // black background

    static const int SX = -2;   // shadow offset left
    static const int SY =  2;   // shadow offset down

    for (int ex : {leftX, rightX}) {
        if (lidH >= _EYE_H) {
            // Fully closed — shadow line then closed line
            d->fillRect(ex + SX, eyeY + _EYE_H - 3 + SY, _EYE_W, 3, true);
            d->fillRect(ex,      eyeY + _EYE_H - 3,       _EYE_W, 3, true);
        } else {
            // Shadow (slightly offset, drawn behind the eye)
            d->fillRoundRect(ex + SX, eyeY + SY, _EYE_W, _EYE_H, _EYE_R, true);

            // Eye body on top
            d->fillRoundRect(ex, eyeY, _EYE_W, _EYE_H, _EYE_R, true);

            // Black lid descending from the top (erases lid portion of eye)
            if (lidH > 0) {
                d->fillRect(ex + SX, eyeY + SY, _EYE_W, lidH, false); // erase shadow lid too
                d->fillRect(ex,      eyeY,       _EYE_W, lidH, false);
            }
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
    // Closing: ease-in (t²) — starts slow, accelerates → feels natural
    // Opening: linear — snaps back quickly
    int lidH = 0;
    if (blink.phase == BLINK_CLOSING) {
        float t = (float)elapsed / BLINK_CLOSE_MS;
        lidH = (int)(t * t * (_EYE_H + 1));   // ease-in quadratic
    } else if (blink.phase == BLINK_OPENING) {
        float t = (float)elapsed / BLINK_OPEN_MS;
        lidH = (int)((1.0f - t) * (_EYE_H + 1));   // linear
    }
    lidH = constrain(lidH, 0, _EYE_H);

    // ── Layout ────────────────────────────────────────────────────────────────
    int totalW = _EYE_W + _SPACE + _EYE_W;
    int leftX  = (screenW - totalW) / 2;
    int rightX = leftX + _EYE_W + _SPACE;
    int eyeY   = (screenH - _EYE_H) / 2;

    _drawEyes(display, leftX, rightX, eyeY, lidH);
}
