#pragma once
#include <face.h>
#include <IDisplay.h>
#include "idle_face.h"
#include "giggle_face.h"

// ─── tara-face — idle + giggle face renderer ─────────────────────────────────
//
// Registers FACE_IDLE and FACE_GIGGLE with the face dispatcher.
// FACE_IDLE — animated blink every 4 s
// FACE_GIGGLE — squinted eyes bounce 4 times, then returns to idle
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   TaraFace face(&display);
//   face.begin();
//
//   void loop() {
//       renderFace(toFaceState(currentState));
//       // or trigger directly:
//       renderFace(FACE_GIGGLE);
//   }

class TaraFace {
public:
    TaraFace(IDisplay* display, int screenW = 128, int screenH = 64);

    void begin();       // registers FACE_IDLE + FACE_GIGGLE
    void drawIdle();
    void drawGiggle();

private:
    IDisplay*    _d;
    int          _sw, _sh;
    BlinkState   _blink;
    GiggleState  _giggle;
};
