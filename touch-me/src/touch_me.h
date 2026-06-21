#pragma once
#include <Arduino.h>
#include <functional>

// ─── touch-me — simple capacitive touch callback ──────────────────────────────
//
// Fires a callback once when a touch is detected. Non-blocking, debounced.
// Supports both rising (value increases on touch) and falling signals.
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   TouchMe touch(32, true);   // GPIO32, value rises when touched
//
//   void setup() {
//       touch.begin();         // measures idle, sets threshold automatically
//       touch.onTouch([]() { Serial.println("touched!"); });
//   }
//
//   void loop() {
//       touch.update();
//   }

using TouchFn = std::function<void()>;

class TouchMe {
public:
    // pin           — GPIO with capacitive touch capability
    // risingOnTouch — true if touchRead() rises when touched
    explicit TouchMe(int pin, bool risingOnTouch = true);

    // Measure idle baseline and set threshold = idle ± margin.
    // Call once in setup() before registering callback.
    void begin(int samples = 20, int margin = 3);

    // Override threshold manually.
    void setThreshold(int threshold, bool risingOnTouch);

    // Set number of consecutive matching reads before state flips (default 3).
    void setDebounce(int count);

    // Register callback — fires once each time a touch is detected.
    void onTouch(TouchFn fn);

    // Call every loop() — non-blocking.
    void update();

    // Accessors
    int  idleValue()  const { return _idleVal; }
    int  threshold()  const { return _threshold; }
    bool isTouched()  const { return _stable; }
    int  rawValue()   const { return (int)touchRead(_pin); }

private:
    int  _pin;
    bool _rising;
    int  _idleVal   = 0;
    int  _threshold = 0;
    int  _debounce  = 3;
    int  _dbc       = 0;
    bool _stable    = false;
    bool _prev      = false;   // previous stable state — fires callback on rising edge only

    TouchFn _onTouch = nullptr;

    bool _isTouched() const;
};
