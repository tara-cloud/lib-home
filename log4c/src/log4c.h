#pragma once
#include <Arduino.h>
#include <functional>
#include <stdarg.h>

// ─── Log levels ───────────────────────────────────────────────────────────────
#define L4C_DEBUG 0
#define L4C_INFO  1
#define L4C_WARN  2
#define L4C_ERROR 3

// ─── Compile-time overrides (define BEFORE #include "log4c.h") ───────────────
// #define LOG4C_CONSOLE_LEVEL L4C_INFO    // suppress DEBUG from Serial
// #define LOG4C_MQTT_LEVEL    L4C_WARN    // only WARN+ over MQTT
// #define LOG4C_QUEUE_SIZE    64
// #define LOG4C_TASK_STACK    4096
// #define LOG4C_TASK_CORE     0

// ─── Macros ───────────────────────────────────────────────────────────────────
#define LLOG(level, fmt, ...) \
    log4c_write(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LDEBUG(fmt, ...) LLOG(L4C_DEBUG, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...)  LLOG(L4C_INFO,  fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...)  LLOG(L4C_WARN,  fmt, ##__VA_ARGS__)
#define LERROR(fmt, ...) LLOG(L4C_ERROR, fmt, ##__VA_ARGS__)

// ─── MQTT publish callback ────────────────────────────────────────────────────
using Log4cPublishFn = std::function<bool(const char* topic, const char* payload)>;

// ─── Init ─────────────────────────────────────────────────────────────────────

// Auto-init: loads baked-in defaults → merges /log4c.json (LittleFS then SPIFFS)
//            → applies any LOG4C_* compile-time defines
void log4c_init();

// Manual override: pass your own JSON config string (same schema as log4c.json)
// Merged on top of defaults; filesystem file is NOT loaded when this is used
void log4c_init(const char* configJson);

// ─── Runtime configuration ────────────────────────────────────────────────────

// Set device name included in every MQTT payload
void log4c_set_device(const String& name);

// Attach (or re-attach) the MQTT appender — safe to call after MQTT connects
// topic   : e.g. "tara01/log"
// publish : your client's publish function; called from drain task (core 0)
void log4c_set_mqtt(const String& topic, Log4cPublishFn publish);

// Generic key/value config setter — dot-notation keys:
//   "console.enabled"  → "true" | "false"
//   "console.level"    → "DEBUG" | "INFO" | "WARN" | "ERROR"
//   "mqtt.enabled"     → "true" | "false"
//   "mqtt.level"       → "DEBUG" | "INFO" | "WARN" | "ERROR"
//   "mqtt.topic"       → "tara01/log"
//   "device"           → "MyDevice"
void log4c_set(const char* key, const char* value);

// Enable/disable an appender at runtime (shorthand)
void log4c_console_enable(bool on);
void log4c_mqtt_enable(bool on);

// Change minimum level for an appender at runtime (shorthand)
void log4c_console_level(int level);
void log4c_mqtt_level(int level);

// ─── Internal — use LLOG macros ──────────────────────────────────────────────
void log4c_write(int level, const char* file, int line, const char* fmt, ...);
