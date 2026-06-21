#include "idle_face.h"
#include <Arduino.h>

// Eye geometry (preferred, user-confirmed)
static const int _IF_EYE_W = 36;
static const int _IF_EYE_H = 36;
static const int _IF_EYE_R = 8;
static const int _IF_SPACE  = 10;

static void _ifDrawOpen(IDisplay* d, int leftX, int rightX, int eyeY) {
    d->fillRoundRect(leftX,  eyeY, _IF_EYE_W, _IF_EYE_H, _IF_EYE_R, true);
    d->fillRoundRect(rightX, eyeY, _IF_EYE_W, _IF_EYE_H, _IF_EYE_R, true);
}

static void _ifDrawClosed(IDisplay* d, int leftX, int rightX, int eyeY) {
    int botY = eyeY + _IF_EYE_H - 3;   // bottom of the eye
    d->fillRect(leftX,  botY, _IF_EYE_W, 3, true);
    d->fillRect(rightX, botY, _IF_EYE_W, 3, true);
}

void renderIdleFace(IDisplay* display, int screenW, int screenH,
                    bool& eyesOpen, unsigned long& lastBlink) {
    unsigned long now = millis();
    if (now - lastBlink >= IDLE_BLINK_INTERVAL) {
        eyesOpen  = !eyesOpen;
        lastBlink = now;
    }

    int totalW = _IF_EYE_W + _IF_SPACE + _IF_EYE_W;
    int leftX  = (screenW - totalW) / 2;
    int rightX = leftX + _IF_EYE_W + _IF_SPACE;
    int eyeY   = (screenH - _IF_EYE_H) / 2;

    display->clear();
    display->fillScreen(false);

    if (eyesOpen)
        _ifDrawOpen(display, leftX, rightX, eyeY);
    else
        _ifDrawClosed(display, leftX, rightX, eyeY);

    display->show();
}
