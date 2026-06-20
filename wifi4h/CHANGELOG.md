# Changelog — wifi4h

## [1.0.0] — 2026-06-20

### Added
- Initial release: WiFi connection + captive-portal hotspot for ESP32
- Consumer-defined field schema via `wifi4h_add_field(name, type, required)`
- Portal form rendered dynamically from registered fields (no static HTML)
- Field types: `"string"`, `"password"`, `"number"` → correct `<input type>` rendered
- Required-field validation on POST — 400 returned if any required field is empty
- All submitted values saved to NVS (`Preferences`) under configurable namespace
- Values persist across reboots; readable via `wifi4h_get(name)` after `wifi4h_load()`
- Built-in `"ssid"` / `"password"` fields drive WiFi connection
- `wifi4h_connect()` — connects to saved SSID or falls back to hotspot
- `wifi4h_reconnect()` — non-blocking reconnect helper for use in `loop()`
- `wifi4h_on_event()` — optional callback for `connected`, `failed`, `ap_start`, `saved`
- `wifi4h_set_namespace()` — isolate NVS namespace per project
- Captive portal probes: iOS (`/hotspot-detect.html`), Android (`/generate_204`), Windows (`/ncsi.txt`, `/connecttest.txt`)
- log4c integration via `LINFO`/`LWARN` macros
