#pragma once
#include <Arduino.h>
#include <functional>

// ─── touch-me — capacitive touch gesture library ──────────────────────────────
//
// Non-blocking gesture detection via ESP32 touchRead().
// Value RISES when touched (unusual but confirmed on this board).
// Threshold is set from a measured idle baseline + margin.
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   #include <touch_me.h>
//
//   TouchMe touch(32);              // GPIO pin
//
//   void setup() {
//       touch.begin();              // measures idle baseline, sets threshold
//
//       touch.on(TOUCH_TAP,         []() { Serial.println("tap"); });
//       touch.on(TOUCH_DOUBLE_TAP,  []() { Serial.println("double tap"); });
//       touch.on(TOUCH_MULTI_TAP,   [](int n) { Serial.printf("x%d\n", n); });
//       touch.on(TOUCH_LONG_PRESS,  []() { Serial.println("hold"); });
//       touch.on(TOUCH_PADDING,     []() { Serial.println("padding"); });
//   }
//
//   void loop() {
//       touch.update();
//   }
//
// ─── Threshold ────────────────────────────────────────────────────────────────
// begin() averages 20 readings as the idle baseline, then:
//   if value rises on touch  → threshold = idle + margin  (default margin=3)
//   if value drops on touch  → threshold = idle - margin
// Call setThreshold(value, rising) to override manually.

// ─── Event constants ──────────────────────────────────────────────────────────
#define TOUCH_TAP         0
#define TOUCH_DOUBLE_TAP  1
#define TOUCH_MULTI_TAP   2   // callback receives tap count as int
#define TOUCH_LONG_PRESS  3
#define TOUCH_PADDING     4   // fires repeatedly while held after long press

using TouchFn     = std::function<void()>;
using TouchCountFn = std::function<void(int)>;   // for TOUCH_MULTI_TAP

class TouchMe {
public:
    // pin       — GPIO with capacitive touch capability
    // risingOnTouch — true if touchRead() RISES when touched (default true)
    explicit TouchMe(int pin, bool risingOnTouch = true);

    // ─── Config ───────────────────────────────────────────────────────────────

    // Measure idle baseline and auto-set threshold.
    // Call once in setup() before registering callbacks.
    void begin(int samples = 20, int margin = 3);

    // Override threshold manually (useful if begin() result is wrong).
    void setThreshold(int threshold, bool risingOnTouch);

    // Timing config (milliseconds)
    void setTapWindow(unsigned long ms);        // max hold time to count as tap  (default 1200)
    void setGapWindow(unsigned long ms);        // multi-tap collection window    (default 600)
    void setLongPressTime(unsigned long ms);    // hold time for long press       (default 3000)
    void setPaddingInterval(unsigned long ms);  // repeat rate while held         (default 400)
    void setDebounce(int count);                // consecutive reads to flip state (default 3)

    // ─── Callbacks ────────────────────────────────────────────────────────────

    void on(int event, TouchFn fn);             // TAP, DOUBLE_TAP, LONG_PRESS, PADDING
    void on(int event, TouchCountFn fn);        // MULTI_TAP (receives count)

    // ─── Loop ─────────────────────────────────────────────────────────────────

    // Call every loop() — non-blocking, millis()-driven.
    void update();

    // ─── Accessors ────────────────────────────────────────────────────────────

    int  idleValue()     const { return _idleVal; }
    int  threshold()     const { return _threshold; }
    bool isTouched()     const { return _stable; }
    int  rawValue()      const { return (int)touchRead(_pin); }

private:
    int  _pin;
    bool _rising;       // true = value rises when touched
    int  _idleVal   = 0;
    int  _threshold = 0;

    // Timing config
    unsigned long _tapWin     = 1200;
    unsigned long _gapWin     = 600;
    unsigned long _longMs     = 3000;
    unsigned long _padMs      = 400;

    // Debounce
    int   _debounce = 3;    // consecutive matching reads to flip state
    int   _dbc      = 0;

    // Gesture state
    bool          _down      = false;
    unsigned long _pressAt   = 0;
    unsigned long _releaseAt = 0;
    int           _tapCount  = 0;
    bool          _longFired = false;
    unsigned long _lastPadAt = 0;

    // Callbacks
    TouchFn      _onTap        = nullptr;
    TouchFn      _onDouble     = nullptr;
    TouchCountFn _onMulti      = nullptr;
    TouchFn      _onLong       = nullptr;
    TouchFn      _onPadding    = nullptr;

    bool _isTouched() const;
};
