#pragma once
#include <IDisplay.h>

// ─── Giggle face ──────────────────────────────────────────────────────────────
// Plays a short giggle animation: eyes squint + bounce up/down 3 times.
// Non-blocking — driven by millis(). Returns true while animating.
// Automatically resets after the animation completes.

// Duration of one bounce cycle (ms)
static const unsigned long GIGGLE_BOUNCE_MS  = 120;
// Number of bounces
static const int           GIGGLE_BOUNCES    = 4;

struct GiggleState {
    bool          active   = false;
    unsigned long startAt  = 0;
    int           bounce   = 0;
};

// Call every loop tick while FACE_GIGGLE.
// Returns true while animating, false when done.
bool renderGiggleFace(IDisplay* display, int screenW, int screenH,
                      GiggleState& state);
