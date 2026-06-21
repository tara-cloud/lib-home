#pragma once
#include <IDisplay.h>

// ─── Idle face — eyes shift position every 3 seconds ─────────────────────────
// No blink. Eyes move to a random offset (left, right, up, down, centre)
// with a smooth 200ms tween transition. Non-blocking.

struct IdleState {
    int           offsetX    = 0;    // current eye offset (tweened)
    int           offsetY    = 0;
    int           targetX    = 0;    // target eye offset
    int           targetY    = 0;
    unsigned long lastMove   = 0;    // when last move was scheduled
    unsigned long moveAt     = 0;    // when next move fires
};

void renderIdleFace(IDisplay* display, int screenW, int screenH,
                    IdleState& state);
