#include "idle_face.h"
#include <Arduino.h>

// Eye geometry — width 30, height 36
static const int _EYE_W = 30;
static const int _EYE_H = 36;
static const int _EYE_R = 8;
static const int _SPACE = 10;

// Shadow offsets
static const int _SX = -2;
static const int _SY =  2;

// Eye move interval and max offset range
static const unsigned long MOVE_INTERVAL = 3000;
static const int           MAX_OFFSET_X  = 4;   // subtle — just 4px left/right
static const int           MAX_OFFSET_Y  = 3;   // subtle — just 3px up/down

static const float TWEEN = 0.08f;   // slow creep — feels like real eye drift

static void _drawEyes(IDisplay* d, int screenW, int screenH, int ox, int oy) {
    int totalW = _EYE_W + _SPACE + _EYE_W;
    int baseX  = (screenW - totalW) / 2;
    int leftX  = baseX  + ox;
    int rightX = baseX + _EYE_W + _SPACE + ox;
    int eyeY   = (screenH - _EYE_H) / 2 + oy;

    d->clear();
    d->fillScreen(false);

    for (int ex : {leftX, rightX}) {
        d->fillRoundRect(ex + _SX, eyeY + _SY, _EYE_W, _EYE_H, _EYE_R, true);  // shadow
        d->fillRoundRect(ex,       eyeY,        _EYE_W, _EYE_H, _EYE_R, true);  // eye
    }

    d->show();
}

// Possible gaze positions: centre + 8 directions
static const int8_t _POSITIONS[][2] = {
    { 0,  0},               // centre (higher weight — appears 3x)
    { 0,  0},
    { 0,  0},
    {-MAX_OFFSET_X,  0},   // left
    { MAX_OFFSET_X,  0},   // right
    { 0, -MAX_OFFSET_Y},   // up
    { 0,  MAX_OFFSET_Y},   // down
    {-MAX_OFFSET_X, -MAX_OFFSET_Y},  // up-left
    { MAX_OFFSET_X, -MAX_OFFSET_Y},  // up-right
};
static const int _N_POSITIONS = sizeof(_POSITIONS) / sizeof(_POSITIONS[0]);

void renderIdleFace(IDisplay* display, int screenW, int screenH,
                    IdleState& state) {
    unsigned long now = millis();

    // Schedule first move
    if (state.moveAt == 0) {
        state.moveAt = now + MOVE_INTERVAL;
    }

    // Fire move — pick random gaze target
    if (now >= state.moveAt) {
        int idx = random(0, _N_POSITIONS);
        state.targetX = _POSITIONS[idx][0];
        state.targetY = _POSITIONS[idx][1];
        state.moveAt  = now + MOVE_INTERVAL + (unsigned long)random(0, 1001);
    }

    // Tween current offset toward target
    float dx = state.targetX - state.offsetX;
    float dy = state.targetY - state.offsetY;
    state.offsetX += (int)(dx * TWEEN);
    state.offsetY += (int)(dy * TWEEN);

    // Snap to target when close enough
    if (abs(state.offsetX - state.targetX) <= 1) state.offsetX = state.targetX;
    if (abs(state.offsetY - state.targetY) <= 1) state.offsetY = state.targetY;

    _drawEyes(display, screenW, screenH, state.offsetX, state.offsetY);
}
