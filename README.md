# lib-home

Shared ESP32/Arduino libraries. Each subdirectory is a standalone PlatformIO library you can reference from any project.

## Usage

In your project's `platformio.ini`:
```ini
lib_extra_dirs = ../lib-home
```

Then `#include` the library header normally.

## Libraries

| Directory | Description |
| --------- | ----------- |
| `log4c` | Structured logging — Serial + async MQTT via FreeRTOS drain task |
