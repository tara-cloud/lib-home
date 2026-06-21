#pragma once
#include <face.h>
#include <IDisplay.h>
#include "idle_face.h"

// ─── tara-face — idle face with animated blink ────────────────────────────────
//
// Delegates idle rendering to idle_face.cpp.
// Blink every 5 s: lid sweeps top→bottom (200 ms) then bottom→top (200 ms).
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   TaraFace face(&display);
//   face.begin();
//
//   void loop() { renderFace(toFaceState(currentState)); }

class TaraFace {
public:
    TaraFace(IDisplay* display, int screenW = 128, int screenH = 64);

    void begin();      // registers FACE_IDLE with the face dispatcher
    void drawIdle();   // delegates to renderIdleFace() in idle_face.cpp

private:
    IDisplay*  _d;
    int        _sw, _sh;
    BlinkState _blink;   // animation state owned here, passed by ref
};
