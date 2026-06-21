#pragma once

// ─── face — face expression declarations for Tara robot display ──────────────
//
// This header is the single catalogue of all supported face expressions.
// Each expression corresponds to a function that must be implemented by the
// consuming project (e.g. device.cpp in tara-robo).
//
// Adding a new expression:
//   1. Declare it here (forward declaration only — no implementation)
//   2. Implement it in device.cpp using u8g2 draw calls or JSON command playback
//   3. Call it from the appropriate state handler in main.cpp / loop()
//
// ─── Idle ─────────────────────────────────────────────────────────────────────

// Animated idle face — plays the TaraExpressions idle animation loop.
// Call continuously in loop() while STATE_IDLE.
void renderIdleFace();
