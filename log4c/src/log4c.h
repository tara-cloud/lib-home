#pragma once
#include <Arduino.h>
#include <stdarg.h>

// ─── Log levels ───────────────────────────────────────────────────────────────
#define L4C_DEBUG 0
#define L4C_INFO  1
#define L4C_WARN  2
#define L4C_ERROR 3

// ─── Macros ───────────────────────────────────────────────────────────────────
// LLOG(level, fmt, ...) — always safe; Serial is immediate, MQTT is async
#define LLOG(level, fmt, ...) \
    log4c_write(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LDEBUG(fmt, ...) LLOG(L4C_DEBUG, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...)  LLOG(L4C_INFO,  fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...)  LLOG(L4C_WARN,  fmt, ##__VA_ARGS__)
#define LERROR(fmt, ...) LLOG(L4C_ERROR, fmt, ##__VA_ARGS__)

// ─── Config ───────────────────────────────────────────────────────────────────
struct Log4cConfig {
    // Minimum level to print to Serial (L4C_DEBUG = all)
    int serialLevel = L4C_DEBUG;

    // Minimum level to send over MQTT (L4C_INFO = skip debug)
    int mqttLevel = L4C_INFO;

    // Queue capacity (entries dropped silently when full)
    int queueSize = 32;

    // FreeRTOS task stack size in bytes
    int taskStackSize = 4096;

    // FreeRTOS task core (0 = protocol/background, 1 = Arduino loop)
    int taskCore = 0;
};

// ─── MQTT sink (optional) ─────────────────────────────────────────────────────
struct Log4cMqtt {
    // Raw publish callback — implement however your project connects to MQTT
    // Called from the drain task (core 0), must be thread-safe
    using PublishFn = std::function<bool(const char* topic, const char* payload)>;

    String    topic;       // e.g. "tara01/log"
    PublishFn publish;     // your MQTT publish function
};

// ─── API ──────────────────────────────────────────────────────────────────────

// Call once at startup (before any LLOG)
// config  — tuning parameters (use defaults if omitted)
// mqtt    — optional; pass nullptr to log to Serial only
void log4c_init(const Log4cConfig& config = Log4cConfig(),
                const Log4cMqtt*   mqtt   = nullptr);

// Set device name included in every MQTT payload
void log4c_set_device(const String& deviceName);

// Change MQTT sink at runtime (e.g. after MQTT connects)
void log4c_set_mqtt(const Log4cMqtt& mqtt);

// Internal — use LLOG macros instead
void log4c_write(int level, const char* file, int line,
                 const char* fmt, ...);
