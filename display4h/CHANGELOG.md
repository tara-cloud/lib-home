# Changelog — display4h

## [1.0.0] — 2026-06-21

### Added

- Initial release: display hardware abstraction for ESP32/Arduino
- `IDisplay` — pure virtual interface (begin, clear, show, primitives, text, contrast)
- `U8g2Display<T>` — template adapter for any U8g2 display driver
- `SSD1306Display` — adapter for Adafruit_SSD1306
- Moved from emo-face/src/ — now a standalone reusable library
