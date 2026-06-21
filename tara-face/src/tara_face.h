#pragma once
#include <face.h>
#include <IDisplay.h>

// ─── tara-face — idle face with 250 ms blink animation ───────────────────────
//
// Draws two white rounded-rectangle eyes centred on screen.
// Every 250 ms the eyes toggle: open ↔ closed (thin horizontal line).
// Non-blocking — driven by millis() inside drawIdle().
//
// Eye size (preferred, user-confirmed):
//   width=36  height=36  radius=8  space=10
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   TaraFace face(&display);
//   face.begin();
//
//   void loop() { renderFace(toFaceState(currentState)); }

class TaraFace {
public:
    TaraFace(IDisplay* display, int screenW = 128, int screenH = 64);

    // Call once in setup() — registers FACE_IDLE with the face dispatcher.
    void begin();

    // Draw idle face — toggles open/closed every 250 ms, non-blocking.
    void drawIdle();

private:
    IDisplay* _d;
    int _sw, _sh;

    // Eye geometry (preferred size)
    static const int EYE_W = 36;
    static const int EYE_H = 36;
    static const int EYE_R = 8;
    static const int SPACE = 10;

    // Blink state
    static const unsigned long BLINK_INTERVAL = 250;
    bool          _eyesOpen   = true;
    unsigned long _lastBlink  = 0;

    void _drawOpen(int leftX, int rightX, int eyeY);
    void _drawClosed(int leftX, int rightX, int eyeY);
};
