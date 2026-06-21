#pragma once
#include <IDisplay.h>

// ─── Eyelid drawing ───────────────────────────────────────────────────────────
// Draws mood-specific eyelid overlays on top of one eye.
// Each eyelid is a black shape that masks part of the white eye:
//
//   TIRED  — black triangle from top-left corner  (inner corner droops down)
//   ANGRY  — black triangle from top-right corner (outer corner drops down)
//   HAPPY  — black rounded rect rising from bottom (cheeks push up)
//
// x, y, w, h  — bounding box of the eye that was just drawn
// isLeft      — true for left eye, false for right (triangle direction mirrors)
// lidTired    — height of tired  eyelid to draw (0 = none, eyeH/2 = full)
// lidAngry    — height of angry  eyelid to draw
// lidHappy    — height of happy  eyelid to draw (bottom offset)
// eyeHDefault — used as the happy overlay rect height
// eyeR        — corner radius (used for happy rounded rect)

void drawEyelids(IDisplay* d,
                 int x, int y, int w, int h,
                 bool isLeft,
                 float lidTired,
                 float lidAngry,
                 float lidHappy,
                 int eyeHDefault,
                 int eyeR);
