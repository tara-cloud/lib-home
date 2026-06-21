#include "touch_me.h"
#include <log4c.h>

TouchMe::TouchMe(int pin, bool risingOnTouch)
    : _pin(pin), _rising(risingOnTouch) {}

void TouchMe::begin(int samples, int margin) {
    long sum = 0;
    for (int i = 0; i < samples; i++) { sum += touchRead(_pin); delay(10); }
    _idleVal   = (int)(sum / samples);
    _threshold = _rising ? _idleVal + margin : _idleVal - margin;
    LINFO("touch-me: GPIO%d  idle=%d  threshold=%d  rising=%s",
          _pin, _idleVal, _threshold, _rising ? "true" : "false");
}

void TouchMe::setThreshold(int threshold, bool risingOnTouch) {
    _threshold = threshold;
    _rising    = risingOnTouch;
}

void TouchMe::setDebounce(int count) { _debounce = count > 0 ? count : 1; }

void TouchMe::onTouch(TouchFn fn) { _onTouch = fn; }

bool TouchMe::_isTouched() const {
    int v = (int)touchRead(_pin);
    return _rising ? v > _threshold : v < _threshold;
}

void TouchMe::update() {
    unsigned long now = millis();

    // Read raw value directly — no debounce for maximum responsiveness
    bool raw = _isTouched();

    // Fire callback when touched AND cooldown has elapsed
    // This catches every touch even if signal is noisy/brief
    if (raw && now - _lastFire >= _cooldown) {
        _lastFire = now;
        if (_onTouch) _onTouch();
        LINFO("touch-me: touched");
    }
}
