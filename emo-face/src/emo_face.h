#pragma once
#include <IDisplay.h>
#include "face.h"

// ─── emo-face — IDisplay-based FaceRenderer implementation ───────────────────
//
// Builds and returns a FaceRenderer populated with expression functions that
// draw directly through IDisplay primitives. No TaraExpressions dependency.
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   #include <face.h>
//   #include <emo_face.h>
//
//   void setup() {
//       face_register(emo_face_renderer(&myDisplay));
//   }
//
//   void loop() {
//       renderFace(FACE_IDLE);          // animates idle
//       renderFace(FACE_WAITING_CONFIG); // draws confused face
//   }
//
// ─── Swapping ────────────────────────────────────────────────────────────────
// Replace with any other renderer factory that returns a FaceRenderer — zero
// changes needed in main.cpp or device.cpp.

// Returns a FaceRenderer with all supported states populated.
// display — any IDisplay adapter (U8g2Display, SSD1306Display, etc.)
FaceRenderer emo_face_renderer(IDisplay* display);
