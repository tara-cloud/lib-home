# Changelog — config4h

## [1.0.0] — 2026-06-20

### Added

- Initial release: device configuration over MQTT for ESP32
- Dedicated MQTT client subscribes to `{projectId}.{deviceName}.config`
- Incoming JSON payload stored in a static `JsonDocument` (always in memory)
- Dot-path accessor `config4h_get("a.b.c")` — walks nested keys, returns `ConfigValue`
- `ConfigValue` typed accessors: `.asBool()`, `.asInt()`, `.asFloat()`, `.asString()` — all accept a default
- `ConfigValue::isNull()` guard for missing keys
- `config4h_on_change(fn)` — optional callback fired on every config update
- `config4h_has_config()` — guard to check if any payload has arrived
- `config4h_raw()` — returns the stored JSON as a String (debug helper)
- Auto-reconnect in `config4h_loop()`
- log4c integration via `LINFO`/`LWARN`/`LDEBUG`/`LERROR` macros
