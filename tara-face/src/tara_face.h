#pragma once
#include <Arduino.h>
#include <face.h>
#include <IDisplay.h>

// ─── Mood constants ───────────────────────────────────────────────────────────
#define TARA_DEFAULT  0
#define TARA_TIRED    1   // drooping inner-top eyelid (sleepy)
#define TARA_ANGRY    2   // sharp outer-top eyelid (angry)
#define TARA_HAPPY    3   // bottom eyelid rises (cheeks up)

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
// RoboEyes-inspired eye renderer. Draws two rounded-rectangle eyes with moods,
// blink, gaze, autoblinker, idle repositioning, flicker, curiosity, and sweat.
// Implements face.h — call begin() to register with the face dispatcher.
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
    void setMood(uint8_t mood);                // TARA_DEFAULT/TIRED/ANGRY/HAPPY
    void setPosition(uint8_t pos);             // TARA_CENTER/N/NE/E/SE/S/SW/W/NW
    void setCuriosity(bool on);                // outer eye grows when looking sideways
    void setCyclops(bool on);                  // one eye only

    // ─── Blink ────────────────────────────────────────────────────────────────
    void close();  void open();  void blink();
    void close(bool left, bool right);
    void open(bool left, bool right);
    void blink(bool left, bool right);

    // ─── Automated animations ────────────────────────────────────────────────
    void setAutoblinker(bool on, int intervalSec = 3, int variationSec = 4);
    void setIdleMode(bool on, int intervalSec = 2, int variationSec = 3);

    // ─── Effects ──────────────────────────────────────────────────────────────
    void setHFlicker(bool on, uint8_t amplitude = 2);   // left/right shiver
    void setVFlicker(bool on, uint8_t amplitude = 5);   // up/down shiver
    void setSweat(bool on);                              // falling sweat drops

    // ─── One-shot animations ──────────────────────────────────────────────────
    void anim_confused();   // shakes left/right for 500 ms
    void anim_laugh();      // bounces up/down for 500 ms

    // ─── Frame rate ───────────────────────────────────────────────────────────
    void setFramerate(uint8_t fps);

    // ─── Draw (called by face dispatcher; also callable directly) ────────────
    void update();     // respects frame rate — call in loop()
    void drawEyes();   // draws immediately, no frame-rate limit

    // ─── Helpers ──────────────────────────────────────────────────────────────
    int getScreenConstraintX();   // max left-eye x before going off-screen
    int getScreenConstraintY();   // max left-eye y before going off-screen

private:
    IDisplay* _d;
    int  _sw, _sh;      // screen width, height

    // ─── Eye defaults (what you set via setWidth/Height/etc.) ────────────────
    float eyeWDefault = 36, eyeHDefault = 36;
    float eyeRDefault = 8;
    int   spaceDef    = 10;

    // ─── Current values — smoothly tweened toward targets each frame ─────────
    float eyeW, eyeH, eyeR;   // eye size + corner radius
    float space;               // space between eyes
    float leftX,  leftY;      // left eye position
    float rightX, rightY;     // right eye position (derived from left)

    // ─── Targets — set by setMood/setPosition/blink/etc. ─────────────────────
    float eyeWt, eyeHt;       // eye size targets
    float leftXt, leftYt;     // gaze position targets

    // ─── Eyelid heights — tweened, 0 = retracted, eyeH/2 = full ─────────────
    float lidTired, lidAngry, lidHappy;    // current (drawn)
    float lidTiredT, lidAngryT, lidHappyT; // targets

    // ─── Mood / flags ─────────────────────────────────────────────────────────
    uint8_t _mood   = TARA_DEFAULT;
    bool curious    = false;
    bool cyclops    = false;

    // ─── Flicker ──────────────────────────────────────────────────────────────
    bool  hFlicker = false; uint8_t hFlickerAmp = 2; bool hFlickerAlt = false;
    bool  vFlicker = false; uint8_t vFlickerAmp = 5; bool vFlickerAlt = false;

    // ─── Sweat drops ──────────────────────────────────────────────────────────
    bool  sweat = false;
    float sweat1y, sweat2y, sweat3y;
    float sweat1h = 1, sweat2h = 1, sweat3h = 1;

    // ─── Autoblinker ──────────────────────────────────────────────────────────
    bool  autoBlink = false;
    int   blinkIntervalSec = 3, blinkVarSec = 4;
    unsigned long blinkTimer = 0;

    // ─── Idle repositioning ───────────────────────────────────────────────────
    bool  idleMode = false;
    int   idleIntervalSec = 2, idleVarSec = 3;
    unsigned long idleTimer = 0;

    // ─── One-shot animation state ─────────────────────────────────────────────
    bool confused = false; unsigned long confusedAt = 0;
    bool laughing = false; unsigned long laughAt    = 0;
    static const int CONFUSED_DUR = 500;
    static const int LAUGH_DUR    = 500;

    // ─── Frame timing ─────────────────────────────────────────────────────────
    int  frameMs = 20;
    unsigned long lastFrameMs = 0;

    // ─── Private helpers ──────────────────────────────────────────────────────
    void  _resetToDefaults();
    void  _drawEyelids(int x, int y, int w, int h, bool isLeft);
    void  _tickSweat();
    float _tween(float current, float target);
    unsigned long _randomMs(int base, int variation);
};
