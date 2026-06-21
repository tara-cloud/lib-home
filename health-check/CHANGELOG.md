# Changelog — health-check

## [1.0.1] — 2026-06-21

### Changed

- Status values replaced with typed constants: `HC_ONLINE`, `HC_HEALTHY`, `HC_ERROR`, `HC_OFFLINE`
- Device-level `status` field now defaults to `HC_ONLINE` ("Online") instead of "Healthy"
- Component `status` field defaults to `HC_HEALTHY` ("Healthy")
- `health_check_set_status()` accepts any status string; use the `HC_*` constants

## [1.0.0] — 2026-06-21

### Added

- Initial release: periodic health-check publisher over MQTT for ESP32
- `health_check_init(host, port, projectId, deviceName)` — dedicated MQTT client, derives topic `{projectId}.{deviceName}.healthcheck`
- `health_check_loop()` — reads `healthcheck.enabled` and `healthcheck.frequency` from config4h on every tick; publishes when interval elapses
- `health_check_publish()` — manual immediate publish regardless of frequency or enabled flag
- `health_check_set_status(componentName, status)` — override status for a named component
- Payload includes `projectId`, `deviceName`, `timestamp`, `status`, `components[]` from reg4h
- Timestamp format: `"YYYY-MM-DD HH:MM:SS.000 IST"` via `getLocalTime()`; falls back to `"uptime:{ms}"` if NTP not synced
- Config changes (frequency, enabled) take effect immediately in the next loop tick
- log4c integration via `LINFO`/`LWARN`/`LERROR` macros
