#pragma once
#include <face.h>
#include <IDisplay.h>
#include "idle_face.h"

// ─── tara-face — idle face with 250 ms blink animation ───────────────────────
//
// Delegates idle rendering to idle_face.cpp — swap that file to change the
// look without touching the class.
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
    IDisplay* _d;
    int _sw, _sh;

    // Blink animation state (passed by reference to renderIdleFace)
    bool          _eyesOpen  = true;
    unsigned long _lastBlink = 0;
};
