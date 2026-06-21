#include "touch_me.h"
#include <log4c.h>

TouchMe::TouchMe(int pin, bool risingOnTouch)
    : _pin(pin), _rising(risingOnTouch) {}

// ─── Config ───────────────────────────────────────────────────────────────────

void TouchMe::begin(int samples, int margin) {
    // Average multiple readings for a stable idle baseline
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += touchRead(_pin);
        delay(10);
    }
    _idleVal   = (int)(sum / samples);
    _threshold = _rising ? _idleVal + margin : _idleVal - margin;

    LINFO("touch-me: GPIO%d  idle=%d  threshold=%d  rising=%s",
          _pin, _idleVal, _threshold, _rising ? "true" : "false");
}

void TouchMe::setThreshold(int threshold, bool risingOnTouch) {
    _threshold = threshold;
    _rising    = risingOnTouch;
}

void TouchMe::setTapWindow(unsigned long ms)      { _tapWin  = ms; }
void TouchMe::setGapWindow(unsigned long ms)      { _gapWin  = ms; }
void TouchMe::setLongPressTime(unsigned long ms)  { _longMs  = ms; }
void TouchMe::setPaddingInterval(unsigned long ms){ _padMs   = ms; }
void TouchMe::setDebounce(int count)              { _debounce = count > 0 ? count : 1; }

// ─── Callbacks ────────────────────────────────────────────────────────────────

void TouchMe::on(int event, TouchFn fn) {
    switch (event) {
        case TOUCH_TAP:        _onTap     = fn; break;
        case TOUCH_DOUBLE_TAP: _onDouble  = fn; break;
        case TOUCH_LONG_PRESS: _onLong    = fn; break;
        case TOUCH_PADDING:    _onPadding = fn; break;
        default: break;
    }
}

void TouchMe::on(int event, TouchCountFn fn) {
    if (event == TOUCH_MULTI_TAP) _onMulti = fn;
}

// ─── Detection ────────────────────────────────────────────────────────────────

bool TouchMe::_isTouched() const {
    int v = (int)touchRead(_pin);
    return _rising ? v > _threshold : v < _threshold;
}

// ─── update ───────────────────────────────────────────────────────────────────

void TouchMe::update() {
    unsigned long now = millis();

    // Debounce — require DEBOUNCE consecutive matching reads before state flips
    bool raw = _isTouched();
    if (raw == _stable) {
        _dbc = 0;
    } else {
        if (++_dbc >= _debounce) {
            _stable = raw;
            _dbc    = 0;
        }
    }

    if (_stable && !_down) {
        // ── Press start ───────────────────────────────────────────────────────
        _down      = true;
        _pressAt   = now;
        _longFired = false;
        _lastPadAt = now;

    } else if (_stable && _down) {
        // ── Still held ────────────────────────────────────────────────────────
        if (!_longFired && now - _pressAt >= _longMs) {
            _longFired = true;
            _tapCount  = 0;   // cancel any pending taps
            if (_onLong) _onLong();
            LINFO("touch-me: long press");
        }
        // Padding — fires repeatedly while held after long press
        if (_longFired && now - _lastPadAt >= _padMs) {
            _lastPadAt = now;
            if (_onPadding) _onPadding();
            LINFO("touch-me: padding");
        }

    } else if (!_stable && _down) {
        // ── Released ──────────────────────────────────────────────────────────
        _down = false;
        if (!_longFired && now - _pressAt < _tapWin) {
            _tapCount++;
            _releaseAt = now;
        }

    } else {
        // ── Not held — resolve tap count after gap window ─────────────────────
        if (_tapCount > 0 && now - _releaseAt >= _gapWin) {
            if (_tapCount == 1) {
                if (_onTap) _onTap();
                LINFO("touch-me: single tap");
            } else if (_tapCount == 2) {
                if (_onDouble) _onDouble();
                LINFO("touch-me: double tap");
            } else {
                if (_onMulti) _onMulti(_tapCount);
                LINFO("touch-me: multi tap (%d)", _tapCount);
            }
            _tapCount = 0;
        }
    }
}
