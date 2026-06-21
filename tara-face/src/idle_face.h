#pragma once
#include <IDisplay.h>

// ─── Idle face render ─────────────────────────────────────────────────────────
// Two white rounded-rectangle eyes centred on screen.
// Blink animation every 5 s:
//   lid descends top→bottom over BLINK_CLOSE_MS,
//   then rises  bottom→top over BLINK_OPEN_MS.
// Non-blocking — all state held in BlinkState.

static const unsigned long BLINK_WAIT_MS  = 5000;  // open hold
static const unsigned long BLINK_CLOSE_MS =  200;  // close duration
static const unsigned long BLINK_OPEN_MS  =  200;  // open duration

enum BlinkPhase { BLINK_WAITING, BLINK_CLOSING, BLINK_OPENING };

struct BlinkState {
    BlinkPhase    phase     = BLINK_WAITING;
    unsigned long phaseAt   = 0;   // millis() when this phase started
};

void renderIdleFace(IDisplay* display, int screenW, int screenH,
                    BlinkState& blink);
