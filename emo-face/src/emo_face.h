#pragma once

// ─── emo-face — TaraExpressions implementation of the face interface ──────────
//
// Implements all declarations from face.h using TaraExpressions + IDisplay.
//
// The consuming project must supply a concrete IDisplay adapter and call
// emo_face_init() once before loop() runs.
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   // In device.cpp / setup():
//   #include <emo_face.h>
//   emo_face_init(&myDisplayAdapter);   // pass any IDisplay*
//
//   // In loop() — driven by face.h contract:
//   renderIdleFace();      // calls TaraExpressions::animateIdle()
//   renderConfusedFace();  // draws confused expression via u8g2 primitives
//
// ─── Swapping implementations ────────────────────────────────────────────────
// To use a different face set, remove emo-face from lib_deps and add a
// different library that also implements face.h. No other changes needed.

#include "IDisplay.h"

// Must be called once before any renderXxxFace() call.
// Stores the IDisplay pointer used by all expression renders.
void emo_face_init(IDisplay* display);
