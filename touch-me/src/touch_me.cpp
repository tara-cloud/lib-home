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
    // Debounce
    bool raw = _isTouched();
    if (raw == _stable) {
        _dbc = 0;
    } else {
        if (++_dbc >= _debounce) { _stable = raw; _dbc = 0; }
    }

    // Fire callback on rising edge (not-touched → touched)
    if (_stable && !_prev) {
        if (_onTouch) _onTouch();
        LINFO("touch-me: touched");
    }
    _prev = _stable;
}
