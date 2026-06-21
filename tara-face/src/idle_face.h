#pragma once
#include <IDisplay.h>

// ─── Idle face render ─────────────────────────────────────────────────────────
// Two white rounded-rectangle eyes centred on screen.
// Blinks every BLINK_INTERVAL ms (open ↔ closed thin line), non-blocking.

static const unsigned long IDLE_BLINK_INTERVAL = 5000;

// Call every loop tick while in FACE_IDLE state.
// display, screenW, screenH — display and dimensions
// eyesOpen, lastBlink       — animation state (persist between calls)
void renderIdleFace(IDisplay* display, int screenW, int screenH,
                    bool& eyesOpen, unsigned long& lastBlink);
