#pragma once
#include <IDisplay.h>

// ─── Sweat drops ──────────────────────────────────────────────────────────────
// Three small animated drops that fall from above the left eye.
// Each drop falls downward and grows taller, then resets to the top.
// Call tick() every frame when sweat is enabled.

struct SweatDrops {
    float y1, y2, y3;   // vertical positions
    float h1 = 1, h2 = 1, h3 = 1;  // heights (grow as they fall)

    // Initialise positions relative to the left eye top-left corner.
    void init(float eyeLeftX, float eyeTopY, int eyeW) {
        y1 = eyeTopY - 4;
        y2 = eyeTopY - 5;
        y3 = eyeTopY - 4;
        _x1 = eyeLeftX;
        _x2 = eyeLeftX + eyeW / 2;
        _x3 = eyeLeftX + eyeW - 2;
        h1 = h2 = h3 = 1;
    }

    // Advance animation one frame and draw the drops.
    // eyeBottomY — y coordinate of the eye's bottom edge (for reset detection).
    void tick(IDisplay* d, float eyeBottomY) {
        y1 += 0.5f;  h1 = constrain(h1 + 0.3f, 1.0f, 4.0f);
        y2 += 0.5f;  h2 = constrain(h2 + 0.3f, 1.0f, 4.0f);
        y3 += 0.5f;  h3 = constrain(h3 + 0.3f, 1.0f, 4.0f);

        if (y1 > eyeBottomY + 4) {   // first drop exits — reset all
            float top = eyeBottomY - 4 - (h1 * 6);  // rough eye top estimate
            y1 = top;      h1 = 1;
            y2 = top - 1;  h2 = 1;
            y3 = top;      h3 = 1;
        }

        d->fillRoundRect((int)_x1, (int)y1, 2, (int)h1, 1, true);
        d->fillRoundRect((int)_x2, (int)y2, 2, (int)h2, 1, true);
        d->fillRoundRect((int)_x3, (int)y3, 2, (int)h3, 1, true);
    }

private:
    float _x1 = 0, _x2 = 0, _x3 = 0;
};
