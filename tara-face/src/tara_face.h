#pragma once
#include <Arduino.h>
#include <face.h>
#include <IDisplay.h>
#include "tara_mood.h"
#include "tara_sweat.h"

// ─── Gaze direction constants ────────────────────────────────────────────────
#define TARA_CENTER   0
#define TARA_N        1
#define TARA_NE       2
#define TARA_E        3
#define TARA_SE       4
#define TARA_S        5
#define TARA_SW       6
#define TARA_W        7
#define TARA_NW       8

// ─── TaraFace ─────────────────────────────────────────────────────────────────
//
// RoboEyes-inspired eye renderer using IDisplay.
// Implements face.h — call begin() to register with the face dispatcher.
//
// File layout:
//   tara_mood.h    — mood constants (TARA_DEFAULT/TIRED/ANGRY/HAPPY), MoodLids struct
//   tara_eyelids.h — drawEyelids() free function
//   tara_sweat.h   — SweatDrops struct
//   tara_face.h/cpp — main engine (this file)
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   TaraFace face(&display, 128, 64, 50);
//   face.setWidth(36, 36);
//   face.setHeight(36, 36);
//   face.setBorderRadius(8, 8);
//   face.setMood(TARA_HAPPY);
//   face.setAutoblinker(true, 3, 4);
//   face.setIdleMode(true, 2, 3);
//   face.begin();
//
//   void loop() { renderFace(toFaceState(currentState)); }

class TaraFace {
public:
    TaraFace(IDisplay* display, int screenW, int screenH, uint8_t fps = 50);

    // ─── Eye shape ────────────────────────────────────────────────────────────
    void setWidth(uint8_t left, uint8_t right);
    void setHeight(uint8_t left, uint8_t right);
    void setBorderRadius(uint8_t left, uint8_t right);
    void setSpaceBetween(int space);

    // ─── Mood & gaze ──────────────────────────────────────────────────────────
    void setMood(uint8_t mood);       // TARA_DEFAULT / TIRED / ANGRY / HAPPY
    void setPosition(uint8_t pos);    // TARA_CENTER / N / NE / E / SE / S / SW / W / NW
    void setCuriosity(bool on);       // outer eye grows when looking sideways
    void setCyclops(bool on);         // one eye only

    // ─── Blink ────────────────────────────────────────────────────────────────
    void close();  void open();  void blink();
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

    // ─── One-shot animations ──────────────────────────────────────────────────
    void anim_confused();   // shakes left/right for 500 ms
    void anim_laugh();      // bounces up/down  for 500 ms

    // ─── Frame rate & draw ────────────────────────────────────────────────────
    void setFramerate(uint8_t fps);
    void update();     // call in loop() — respects frame rate
    void drawEyes();   // draw immediately (no rate limit)

    // ─── Helpers ──────────────────────────────────────────────────────────────
    int getScreenConstraintX();
    int getScreenConstraintY();

    // ─── Register with face.h dispatcher ─────────────────────────────────────
    void begin();

private:
    IDisplay* _d;
    int  _sw, _sh;

    // ─── Eye defaults (configured via setters) ────────────────────────────────
    float eyeWDefault = 36, eyeHDefault = 36;
    float eyeRDefault = 8;
    int   spaceDef    = 10;

    // ─── Current geometry (tweened each frame toward targets) ─────────────────
    float eyeW, eyeH, eyeR, space;
    float leftX, leftY, rightX, rightY;

    // ─── Targets ──────────────────────────────────────────────────────────────
    float eyeWt, eyeHt;
    float leftXt, leftYt;

    // ─── Mood (eyelids) ───────────────────────────────────────────────────────
    MoodLids _lids;

    // ─── Sweat drops ──────────────────────────────────────────────────────────
    SweatDrops _sweatDrops;
    bool sweat = false;

    // ─── Flags ────────────────────────────────────────────────────────────────
    bool curious = false;
    bool cyclops = false;

    // ─── Flicker ──────────────────────────────────────────────────────────────
    bool  hFlicker = false; uint8_t hFlickerAmp = 2; bool hFlickerAlt = false;
    bool  vFlicker = false; uint8_t vFlickerAmp = 5; bool vFlickerAlt = false;

    // ─── Autoblinker ──────────────────────────────────────────────────────────
    bool autoBlink = false;
    int  blinkIntervalSec = 3, blinkVarSec = 4;
    unsigned long blinkTimer = 0;

    // ─── Idle repositioning ───────────────────────────────────────────────────
    bool idleMode = false;
    int  idleIntervalSec = 2, idleVarSec = 3;
    unsigned long idleTimer = 0;

    // ─── One-shot animations ──────────────────────────────────────────────────
    bool confused = false; unsigned long confusedAt = 0;
    bool laughing = false; unsigned long laughAt    = 0;
    static const int CONFUSED_DUR = 500;
    static const int LAUGH_DUR    = 500;

    // ─── Frame timing ─────────────────────────────────────────────────────────
    int frameMs = 20;
    unsigned long lastFrameMs = 0;

    // ─── Private helpers ──────────────────────────────────────────────────────
    void  _resetToDefaults();
    float _tween(float current, float target);
    unsigned long _randomMs(int base, int variation);
};
