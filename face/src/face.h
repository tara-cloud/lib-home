#pragma once
#include <Arduino.h>

// ─── face — face expression interface ────────────────────────────────────────
//
// Forward declarations for all supported face expressions.
// Each function must be implemented by exactly ONE face implementation library
// (e.g. emo-face). The consuming project includes both this header and the
// chosen implementation library in platformio.ini lib_deps.
//
// Swapping implementation:
//   Replace the implementation library in lib_deps — nothing in main.cpp or
//   device.cpp changes. The linker picks up the new definitions automatically.
//
// Adding a new expression:
//   1. Declare it here
//   2. Implement it in the chosen implementation library
//
// ─── Declarations ────────────────────────────────────────────────────────────

// Animated idle face — call continuously in loop() while STATE_IDLE.
void renderIdleFace();

// Confused / waiting face — call while STATE_WAITING_CONFIG.
void renderConfusedFace();
