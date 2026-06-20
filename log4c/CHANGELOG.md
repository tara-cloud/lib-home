# Changelog — log4c

## [1.0.0] — 2026-06-20

### Added
- `log4c_init()` — initialises queue, starts FreeRTOS drain task on core 0
- `log4c_set_device()` — sets device name included in every MQTT payload
- `log4c_set_mqtt()` — attaches MQTT sink at runtime (thread-safe, call after connect)
- `LDEBUG / LINFO / LWARN / LERROR` macros
- `Log4cConfig` — tunable queue size, task stack, log levels, task core
- `Log4cMqtt` — publish callback abstraction (works with any MQTT client)
- Serial always prints immediately; MQTT drains asynchronously at ~20 msg/s
