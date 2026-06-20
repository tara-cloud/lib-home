#include "log4c.h"
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// ─── Internal types ───────────────────────────────────────────────────────────

static const char* LEVEL_NAMES[] = { "DEBUG", "INFO", "WARN", "ERROR" };

struct Entry {
    int      level;
    uint32_t ms;
    char     logger[32];
    char     message[220];
};

// ─── State ────────────────────────────────────────────────────────────────────

static Log4cConfig        _cfg;
static Log4cMqtt          _mqtt;
static bool               _mqttSet   = false;
static String             _device;
static SemaphoreHandle_t  _mutex     = nullptr;
static Entry*             _queue     = nullptr;
static int                _head      = 0;
static int                _tail      = 0;
static int                _count     = 0;

// ─── Queue helpers ────────────────────────────────────────────────────────────

static void _enqueue(const Entry& e) {
    if (!_mutex || !_queue) return;
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(5)) != pdTRUE) return;
    if (_count < _cfg.queueSize) {
        _queue[_tail] = e;
        _tail = (_tail + 1) % _cfg.queueSize;
        _count++;
    }
    xSemaphoreGive(_mutex);
}

static bool _dequeue(Entry& out) {
    if (!_mutex || _count == 0) return false;
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) != pdTRUE) return false;
    bool ok = _count > 0;
    if (ok) {
        out   = _queue[_head];
        _head = (_head + 1) % _cfg.queueSize;
        _count--;
    }
    xSemaphoreGive(_mutex);
    return ok;
}

// ─── Drain task ───────────────────────────────────────────────────────────────

static void _drainTask(void*) {
    for (;;) {
        Entry e;
        if (_mqttSet && _dequeue(e)) {
            JsonDocument doc;
            doc["level"]     = LEVEL_NAMES[e.level];
            doc["logger"]    = e.logger;
            doc["message"]   = e.message;
            doc["timestamp"] = e.ms;
            if (_device.length()) doc["device"] = _device;

            String out;
            serializeJson(doc, out);
            _mqtt.publish(_mqtt.topic.c_str(), out.c_str());
            vTaskDelay(pdMS_TO_TICKS(50)); // ~20 msg/s
        } else {
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

// ─── Public API ───────────────────────────────────────────────────────────────

void log4c_init(const Log4cConfig& config, const Log4cMqtt* mqtt) {
    _cfg   = config;
    _mutex = xSemaphoreCreateMutex();
    _queue = new Entry[_cfg.queueSize];

    if (mqtt) {
        _mqtt    = *mqtt;
        _mqttSet = true;
    }

    xTaskCreatePinnedToCore(
        _drainTask, "log4c",
        _cfg.taskStackSize, nullptr,
        1, nullptr,
        _cfg.taskCore
    );
}

void log4c_set_device(const String& deviceName) {
    _device = deviceName;
}

void log4c_set_mqtt(const Log4cMqtt& mqtt) {
    if (_mutex) xSemaphoreTake(_mutex, portMAX_DELAY);
    _mqtt    = mqtt;
    _mqttSet = true;
    if (_mutex) xSemaphoreGive(_mutex);
}

void log4c_write(int level, const char* file, int line,
                 const char* fmt, ...) {
    char buf[220];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    // Serial — always immediate
    if (level >= _cfg.serialLevel) {
        const char* slash = strrchr(file, '/');
        if (!slash) slash = strrchr(file, '\\');
        const char* fname = slash ? slash + 1 : file;
        Serial.printf("[%s] %s:%d %s\n", LEVEL_NAMES[level], fname, line, buf);
    }

    // MQTT queue — only if level qualifies
    if (_mqttSet && level >= _cfg.mqttLevel) {
        Entry e;
        e.level = level;
        e.ms    = millis();
        const char* slash = strrchr(file, '/');
        if (!slash) slash = strrchr(file, '\\');
        strncpy(e.logger, slash ? slash + 1 : file, sizeof(e.logger) - 1);
        e.logger[sizeof(e.logger) - 1] = 0;
        strncpy(e.message, buf, sizeof(e.message) - 1);
        e.message[sizeof(e.message) - 1] = 0;
        _enqueue(e);
    }
}
