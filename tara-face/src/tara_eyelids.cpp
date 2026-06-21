#include "tara_eyelids.h"

void drawEyelids(IDisplay* d,
                 int x, int y, int w, int h,
                 bool isLeft,
                 float lidTired,
                 float lidAngry,
                 float lidHappy,
                 int eyeHDefault,
                 int eyeR) {

    // TIRED — drooping inner-top corner
    // Left eye:  triangle anchored at top-left,  tip points down-left
    // Right eye: triangle anchored at top-right, tip points down-right
    if (lidTired > 0) {
        int lh = (int)lidTired;
        if (isLeft)
            d->fillTriangle(x,     y - 1,  x + w, y - 1,  x,     y + lh - 1, false);
        else
            d->fillTriangle(x,     y - 1,  x + w, y - 1,  x + w, y + lh - 1, false);
    }

    // ANGRY — sharp outer-top corner (mirrors tired)
    // Left eye:  tip points down-right (inner side)
    // Right eye: tip points down-left  (inner side)
    if (lidAngry > 0) {
        int lh = (int)lidAngry;
        if (isLeft)
            d->fillTriangle(x,     y - 1,  x + w, y - 1,  x + w, y + lh - 1, false);
        else
            d->fillTriangle(x,     y - 1,  x + w, y - 1,  x,     y + lh - 1, false);
    }

    // HAPPY — bottom eyelid rises up (rounded rect from bottom edge)
    if (lidHappy > 0) {
        int offset = (int)lidHappy;
        d->fillRoundRect(x - 1,  (y + h) - offset + 1,  w + 2,  eyeHDefault,  eyeR,  false);
    }
}
