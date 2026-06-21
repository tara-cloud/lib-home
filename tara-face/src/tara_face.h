#pragma once
#include <face.h>
#include <IDisplay.h>

// ─── tara-face — minimal idle face renderer ───────────────────────────────────
//
// Draws two white rounded-rectangle eyes centred on screen.
// Implements face.h — call begin() to register with the face dispatcher.
//
// Eye size (preferred, user-confirmed):
//   width=36  height=36  radius=8  space=10
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   TaraFace face(&display, 128, 64);
//   face.begin();
//
//   void loop() { renderFace(toFaceState(currentState)); }

class TaraFace {
public:
    TaraFace(IDisplay* display, int screenW = 128, int screenH = 64);

    // Call once in setup() — registers FACE_IDLE with the face dispatcher.
    void begin();

    // Draw idle face immediately (two centred white eyes).
    void drawIdle();

private:
    IDisplay* _d;
    int _sw, _sh;

    // Eye geometry (preferred size)
    static const int EYE_W = 36;
    static const int EYE_H = 36;
    static const int EYE_R = 8;
    static const int SPACE = 10;
};
