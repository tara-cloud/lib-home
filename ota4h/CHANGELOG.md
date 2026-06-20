# Changelog — ota4h

## [1.0.0] — 2026-06-20

### Added
- Initial release: OTA firmware update over MQTT for ESP32/Arduino
- Dedicated MQTT client — independent from application's main client
- Inbound topic `{projectId}.{deviceName}.ota` — accepts `{ "version", "url", "apiKey" }`
- Outbound topic `{projectId}.{deviceName}.ota_status` — publishes `downloading | progress | ok | failed`
- Progress reporting every 10% via `httpUpdate.onProgress`
- Optional `ota4h_on_state(fn)` callback for UI/state-machine integration
- Automatic log4c MQTT appender wiring on connect
- `ota4h_init(host, port, projectId, deviceName)` — fully self-contained, no TaraCore globals
- Auto-reconnect in `ota4h_loop()`
