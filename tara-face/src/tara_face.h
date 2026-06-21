#pragma once
#include <Arduino.h>
#include <face.h>
#include <IDisplay.h>

// ─── tara-face — RoboEyes-inspired eye renderer using IDisplay ────────────────
//
// Port of FluxGarage/RoboEyes adapted for the IDisplay abstraction layer.
// Supports moods, blink, autoblinker, idle repositioning, flicker, curiosity,
// cyclops mode, sweat drops, and one-shot animations.
//
// Implements face.h — call begin() to register with the face dispatcher.
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   TaraFace face(&display, 128, 64, 50);  // display, w, h, fps
//
//   face.setWidth(36, 36);
//   face.setHeight(36, 36);
//   face.setBorderRadius(8, 8);
//   face.setMood(TARA_HAPPY);
//   face.setAutoblinker(true, 3, 4);
//   face.setIdleMode(true, 2, 3);
//   face.begin();          // registers with face.h dispatcher
//
//   void loop() {
//       renderFace(toFaceState(currentState));
//   }

// ─── Mood constants ───────────────────────────────────────────────────────────
#define TARA_DEFAULT  0
#define TARA_TIRED    1
#define TARA_ANGRY    2
#define TARA_HAPPY    3

// ─── Eye position presets ────────────────────────────────────────────────────
#define TARA_CENTER   0
#define TARA_N        1
#define TARA_NE       2
#define TARA_E        3
#define TARA_SE       4
#define TARA_S        5
#define TARA_SW       6
#define TARA_W        7
#define TARA_NW       8

class TaraFace {
public:
    // ─── Init ─────────────────────────────────────────────────────────────────
    TaraFace(IDisplay* display, int screenW, int screenH, uint8_t fps = 50);

    // Build FaceRenderer and call face_register(). Call once in setup().
    void begin();

    // ─── Eye shape ────────────────────────────────────────────────────────────
    void setWidth(uint8_t left, uint8_t right);
    void setHeight(uint8_t left, uint8_t right);
    void setBorderRadius(uint8_t left, uint8_t right);
    void setSpaceBetween(int space);

    // ─── Mood & expression ────────────────────────────────────────────────────
    void setMood(uint8_t mood);
    void setPosition(uint8_t position);
    void setCuriosity(bool on);   // outer eye grows when looking left/right
    void setCyclops(bool on);     // draw only one (left) eye

    // ─── Blink control ────────────────────────────────────────────────────────
    void close();
    void open();
    void blink();
    void close(bool left, bool right);
    void open(bool left, bool right);
    void blink(bool left, bool right);

    // ─── Automated animations ────────────────────────────────────────────────
    void setAutoblinker(bool on, int intervalSec = 3, int variationSec = 4);
    void setIdleMode(bool on, int intervalSec = 2, int variationSec = 3);

    // ─── Effects ──────────────────────────────────────────────────────────────
    void setHFlicker(bool on, uint8_t amplitude = 2);
    void setVFlicker(bool on, uint8_t amplitude = 5);
    void setSweat(bool on);

    // ─── One-shot macro animations ────────────────────────────────────────────
    void anim_confused();   // shake left/right
    void anim_laugh();      // shake up/down

    // ─── Frame rate ───────────────────────────────────────────────────────────
    void setFramerate(uint8_t fps);

    // ─── Draw (called by face dispatcher; also callable directly) ────────────
    void update();
    void drawEyes();

    // ─── Screen constraint helpers ────────────────────────────────────────────
    int getScreenConstraintX();
    int getScreenConstraintY();

private:
    IDisplay* _d;
    int  _sw, _sh;           // screen dimensions
    int  _frameInterval;     // ms per frame
    unsigned long _fpsTimer = 0;

    // ─── Eye geometry defaults ────────────────────────────────────────────────
    int  _eyeLwD = 36, _eyeLhD = 36; uint8_t _eyeLrD = 8;
    int  _eyeRwD = 36, _eyeRhD = 36; uint8_t _eyeRrD = 8;
    int  _spaceBetweenD = 10;

    // ─── Current (tweened) geometry ───────────────────────────────────────────
    float _eyeLwC, _eyeLhC; float _eyeLrC;
    float _eyeRwC, _eyeRhC; float _eyeRrC;
    float _spaceC;

    // ─── Target geometry (tweening targets) ──────────────────────────────────
    float _eyeLwN, _eyeLhN;
    float _eyeRwN, _eyeRhN;

    // ─── Eye positions ────────────────────────────────────────────────────────
    float _eyeLx, _eyeLy, _eyeRx, _eyeRy;
    float _eyeLxN, _eyeLyN;

    // ─── Blink state ──────────────────────────────────────────────────────────
    bool  _eyeLclosed = false, _eyeRclosed = false;

    // ─── Eyelid heights (tweened) ─────────────────────────────────────────────
    float _lidTiredL  = 0, _lidTiredLN  = 0;
    float _lidTiredR  = 0, _lidTiredRN  = 0;
    float _lidAngryL  = 0, _lidAngryLN  = 0;
    float _lidAngryR  = 0, _lidAngryRN  = 0;
    float _lidHappyL  = 0, _lidHappyLN  = 0;
    float _lidHappyR  = 0, _lidHappyRN  = 0;

    // ─── Mood ─────────────────────────────────────────────────────────────────
    uint8_t _mood = TARA_DEFAULT;

    // ─── Curiosity ────────────────────────────────────────────────────────────
    bool  _curious = false;
    float _eyeLhOffset = 0, _eyeRhOffset = 0;

    // ─── Cyclops ──────────────────────────────────────────────────────────────
    bool _cyclops = false;

    // ─── Flicker ──────────────────────────────────────────────────────────────
    bool  _hFlicker = false; uint8_t _hFlickerAmp = 2; bool _hFlickerAlt = false;
    bool  _vFlicker = false; uint8_t _vFlickerAmp = 5; bool _vFlickerAlt = false;

    // ─── Sweat ────────────────────────────────────────────────────────────────
    bool  _sweat = false;
    float _sweat1x, _sweat1y, _sweat1w = 1, _sweat1h = 1;
    float _sweat2x, _sweat2y, _sweat2w = 1, _sweat2h = 1;
    float _sweat3x, _sweat3y, _sweat3w = 1, _sweat3h = 1;

    // ─── Autoblinker ──────────────────────────────────────────────────────────
    bool  _autoBlink = false;
    int   _blinkIntervalSec = 3, _blinkVarSec = 4;
    unsigned long _blinkTimer = 0;

    // ─── Idle mode ────────────────────────────────────────────────────────────
    bool  _idle = false;
    int   _idleIntervalSec = 2, _idleVarSec = 3;
    unsigned long _idleTimer = 0;

    // ─── One-shot animations ──────────────────────────────────────────────────
    bool  _confused = false; unsigned long _confusedStart = 0;
    static const int _CONFUSED_DUR = 500;
    bool  _laughing = false; unsigned long _laughStart = 0;
    static const int _LAUGH_DUR = 500;

    // ─── Helpers ──────────────────────────────────────────────────────────────
    void  _initPositions();
    void  _applyMoodTargets();
    void  _applyPosition(uint8_t pos);
    float _tween(float cur, float target);
    void  _drawSweat();
};
