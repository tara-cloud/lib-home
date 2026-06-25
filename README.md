# lib-home

Index of shared ESP32/Arduino libraries for the Tara platform.  
Each library lives in its own repository under the [tara-cloud](https://github.com/tara-cloud) org.

---

## Libraries

| Library | Description | Repo |
| --- | --- | --- |
| [config4h](https://github.com/tara-cloud/config4h) | Device configuration over MQTT — JSON document, dot-path access, live callbacks | [tara-cloud/config4h](https://github.com/tara-cloud/config4h) |
| [display4h](https://github.com/tara-cloud/display4h) | Display hardware abstraction — `IDisplay` interface, U8g2 and SSD1306 adapters | [tara-cloud/display4h](https://github.com/tara-cloud/display4h) |
| [emo-face](https://github.com/tara-cloud/emo-face) | IDisplay-based FaceRenderer — animated eyes, implements `face.h` contract | [tara-cloud/emo-face](https://github.com/tara-cloud/emo-face) |
| [face](https://github.com/tara-cloud/face) | Header-only face expression catalogue — forward declarations for all display expressions | [tara-cloud/face](https://github.com/tara-cloud/face) |
| [health-check](https://github.com/tara-cloud/health-check) | Periodic health-check publisher over MQTT — reads config from config4h, components from reg4h | [tara-cloud/health-check](https://github.com/tara-cloud/health-check) |
| [log4c](https://github.com/tara-cloud/log4c) | Multi-appender structured logging — Console + MQTT, JSON config, FreeRTOS drain task | [tara-cloud/log4c](https://github.com/tara-cloud/log4c) |
| [ota4h](https://github.com/tara-cloud/ota4h) | OTA-over-MQTT firmware update manager — dedicated MQTT client, progress reporting | [tara-cloud/ota4h](https://github.com/tara-cloud/ota4h) |
| [reg4h](https://github.com/tara-cloud/reg4h) | Component registry — unified I2C scan + GPIO registration, read-back accessors | [tara-cloud/reg4h](https://github.com/tara-cloud/reg4h) |
| [tara-face](https://github.com/tara-cloud/tara-face) | RoboEyes-inspired eye renderer — moods, blink, autoblinker, idle, flicker, curiosity | [tara-cloud/tara-face](https://github.com/tara-cloud/tara-face) |
| [touch-me](https://github.com/tara-cloud/touch-me) | Capacitive touch gesture library — tap, double-tap, long press, non-blocking | [tara-cloud/touch-me](https://github.com/tara-cloud/touch-me) |
| [wifi4h](https://github.com/tara-cloud/wifi4h) | WiFi + captive-portal hotspot — consumer-defined field schema, NVS persistence | [tara-cloud/wifi4h](https://github.com/tara-cloud/wifi4h) |
