# Changelog — reg4h

## [1.0.0] — 2026-06-21

### Added

- Initial release: unified component registry for ESP32
- Single `reg4h_add_component(name, type, protocol, pins[], pinCount)` API for all hardware
- `protocol == "I2C"` — scans `pins[0]` (SDA) / `pins[1]` (SCL) bus, adds one entry per found address, matches KNOWN_I2C table (OLED, MPU6050, BME280, INA219, LCD); unknown addresses use caller's name/type fallback
- `protocol == other` — stores one component; pins stored as `"GPIO{n}"`, label `"PIN{n}"`, direction derived from type
- `Wire.end()` called after I2C scan so display/peripheral init can take the bus next
- `reg4h_component_count()` / `reg4h_get_component(i)` read-back accessors for registration payloads
- log4c integration via `LINFO`/`LDEBUG`/`LWARN` macros
- No shared globals — all state is internal static
