# Changelog — log4c

## [2.0.0] — 2026-06-20

### Changed (breaking)
- All logging goes through a single queue — main thread never blocks
- Console appender now also queued; FreeRTOS drain task handles both Serial and MQTT
- `log4c_init(config, mqtt*)` replaced by `log4c_init()` — config auto-loaded
- `Log4cConfig` / `Log4cMqtt` structs removed

### Added
- Multi-appender: Console (Serial) + MQTT, independently enabled/levelled
- JSON config loaded from `/log4c.json` (LittleFS → SPIFFS fallback)
- Baked-in default config (works with no filesystem)
- Compile-time overrides: `LOG4C_CONSOLE_LEVEL`, `LOG4C_MQTT_LEVEL`, `LOG4C_QUEUE_SIZE`
- `log4c_init(const char* json)` — inline JSON override
- `log4c_console_enable/level()`, `log4c_mqtt_enable/level()` — runtime control
- `log4c/data/log4c.json` — sample config to copy to project `data/`

---

## [1.0.0] — 2026-06-20

### Added
- Initial release: Serial + MQTT, FreeRTOS drain task, `LDEBUG/LINFO/LWARN/LERROR`
