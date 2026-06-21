# Changelog — touch-me

## [1.0.0] — 2026-06-21

### Added

- Initial release: capacitive touch gesture library for ESP32
- `TouchMe(pin, risingOnTouch)` — construct with GPIO pin; risingOnTouch=true if value rises on contact
- `begin(samples, margin)` — measures idle baseline, auto-sets threshold
- `setThreshold(value, rising)` — manual override
- `on(TOUCH_TAP, fn)` — single tap callback
- `on(TOUCH_DOUBLE_TAP, fn)` — double tap callback
- `on(TOUCH_MULTI_TAP, fn(int))` — 3+ taps, callback receives count
- `on(TOUCH_LONG_PRESS, fn)` — fires once after hold >= longPressTime
- `on(TOUCH_PADDING, fn)` — fires repeatedly while held after long press
- `setTapWindow(ms)`, `setGapWindow(ms)`, `setLongPressTime(ms)`, `setPaddingInterval(ms)` — timing config
- `isTouched()`, `rawValue()`, `idleValue()`, `threshold()` — state accessors
- Debounce: 3 consecutive matching reads required to flip state
- Non-blocking — millis()-driven, call `update()` in loop()
- log4c integration via LINFO macros
