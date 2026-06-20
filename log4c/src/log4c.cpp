#include "log4c.h"
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// ─── Filesystem (LittleFS → SPIFFS fallback) ─────────────────────────────────
#ifdef ESP32
  #if __has_include(<LittleFS.h>)
    #include <LittleFS.h>
    #define L4C_FS LittleFS
    #define L4C_FS_LITTLEFS
  #elif __has_include(<SPIFFS.h>)
    #include <SPIFFS.h>
    #define L4C_FS SPIFFS
  #endif
#endif

// ─── Default config (baked in — works with no filesystem) ─────────────────────
static const char DEFAULT_CONFIG[] = R"({
  "appenders": {
    "console": { "enabled": true,  "level": "DEBUG" },
    "mqtt":    { "enabled": false, "level": "INFO",  "topic": "" }
  },
  "queue":  { "size": 64 },
  "task":   { "stackSize": 4096, "core": 0 }
})";

// ─── Helpers ──────────────────────────────────────────────────────────────────
static const char* LEVEL_NAMES[] = { "DEBUG", "INFO", "WARN", "ERROR" };

static int _levelFromStr(const char* s) {
    if (!s)              return L4C_DEBUG;
    if (!strcmp(s, "INFO"))  return L4C_INFO;
    if (!strcmp(s, "WARN"))  return L4C_WARN;
    if (!strcmp(s, "ERROR")) return L4C_ERROR;
    return L4C_DEBUG;
}

// ─── Appender state ───────────────────────────────────────────────────────────
static bool   _consoleEnabled = true;
static int    _consoleLevel   = L4C_DEBUG;
static bool   _mqttEnabled    = false;
static int    _mqttLevel      = L4C_INFO;
static String _mqttTopic;
static Log4cPublishFn _mqttPublish;

// ─── Queue ────────────────────────────────────────────────────────────────────
struct Entry {
    int      level;
    uint32_t ms;
    char     logger[32];
    char     message[220];
};

static Entry*            _queue    = nullptr;
static int               _qSize    = 64;
static int               _head     = 0;
static int               _tail     = 0;
static int               _count    = 0;
static volatile uint32_t _dropped  = 0;   // entries lost when queue was full
static SemaphoreHandle_t _mutex    = nullptr;

static String _device;
static int    _taskStack = 4096;
static int    _taskCore  = 0;

// enqueue is called from any task — never blocks
static void _enqueue(const Entry& e) {
    if (!_mutex || !_queue) return;
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(5)) != pdTRUE) {
        _dropped++;
        return;
    }
    if (_count < _qSize) {
        _queue[_tail] = e;
        _tail = (_tail + 1) % _qSize;
        _count++;
    } else {
        _dropped++;   // queue full — track but never block
    }
    xSemaphoreGive(_mutex);
}

static bool _dequeue(Entry& out) {
    if (!_mutex || _count == 0) return false;
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) != pdTRUE) return false;
    bool ok = (_count > 0);
    if (ok) {
        out   = _queue[_head];
        _head = (_head + 1) % _qSize;
        _count--;
    }
    xSemaphoreGive(_mutex);
    return ok;
}

// ─── Drain task ───────────────────────────────────────────────────────────────
static void _drainTask(void*) {
    for (;;) {
        // Report any dropped entries first
        if (_dropped > 0) {
            uint32_t d = _dropped;
            _dropped   = 0;
            Serial.printf("[WARN] log4c: %u entries dropped (queue full)\n", d);
        }

        Entry e;
        if (_dequeue(e)) {
            // Console appender
            if (_consoleEnabled && e.level >= _consoleLevel) {
                Serial.printf("[%s] %s %s\n",
                    LEVEL_NAMES[e.level], e.logger, e.message);
            }

            // MQTT appender
            if (_mqttEnabled && _mqttPublish && e.level >= _mqttLevel) {
                JsonDocument doc;
                doc["level"]     = LEVEL_NAMES[e.level];
                doc["logger"]    = e.logger;
                doc["message"]   = e.message;
                doc["timestamp"] = e.ms;
                if (_device.length()) doc["device"] = _device;
                String out;
                serializeJson(doc, out);
                _mqttPublish(_mqttTopic.c_str(), out.c_str());
                vTaskDelay(pdMS_TO_TICKS(20)); // brief yield after MQTT publish
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10)); // idle — wake quickly for next entry
        }
    }
}

// ─── Config loader ────────────────────────────────────────────────────────────
static void _applyJson(const char* json) {
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) return;

    if (!doc["appenders"]["console"]["enabled"].isNull())
        _consoleEnabled = doc["appenders"]["console"]["enabled"].as<bool>();
    if (!doc["appenders"]["console"]["level"].isNull())
        _consoleLevel = _levelFromStr(doc["appenders"]["console"]["level"]);

    if (!doc["appenders"]["mqtt"]["enabled"].isNull())
        _mqttEnabled = doc["appenders"]["mqtt"]["enabled"].as<bool>();
    if (!doc["appenders"]["mqtt"]["level"].isNull())
        _mqttLevel = _levelFromStr(doc["appenders"]["mqtt"]["level"]);
    if (!doc["appenders"]["mqtt"]["topic"].isNull())
        _mqttTopic = doc["appenders"]["mqtt"]["topic"].as<String>();

    if (!doc["queue"]["size"].isNull())     _qSize     = doc["queue"]["size"].as<int>();
    if (!doc["task"]["stackSize"].isNull()) _taskStack = doc["task"]["stackSize"].as<int>();
    if (!doc["task"]["core"].isNull())      _taskCore  = doc["task"]["core"].as<int>();
}

static void _loadFromFilesystem() {
#ifdef L4C_FS
    bool mounted = false;

    #ifdef L4C_FS_LITTLEFS
    if (LittleFS.begin(false)) mounted = true;
    #endif

    #if !defined(L4C_FS_LITTLEFS) && __has_include(<SPIFFS.h>)
    if (!mounted && SPIFFS.begin(false)) mounted = true;
    #endif

    if (!mounted) return;

    if (L4C_FS.exists("/log4c.json")) {
        File f = L4C_FS.open("/log4c.json", "r");
        if (f) {
            String s = f.readString();
            f.close();
            _applyJson(s.c_str());
        }
    }
#endif
}

static void _applyCompileTimeOverrides() {
#ifdef LOG4C_CONSOLE_LEVEL
    _consoleLevel = LOG4C_CONSOLE_LEVEL;
#endif
#ifdef LOG4C_MQTT_LEVEL
    _mqttLevel = LOG4C_MQTT_LEVEL;
#endif
#ifdef LOG4C_QUEUE_SIZE
    _qSize = LOG4C_QUEUE_SIZE;
#endif
#ifdef LOG4C_TASK_STACK
    _taskStack = LOG4C_TASK_STACK;
#endif
#ifdef LOG4C_TASK_CORE
    _taskCore = LOG4C_TASK_CORE;
#endif
}

static void _start() {
    _mutex = xSemaphoreCreateMutex();
    _queue = new Entry[_qSize];
    xTaskCreatePinnedToCore(_drainTask, "log4c",
        _taskStack, nullptr, 2, nullptr, _taskCore);
}

// ─── Public API ───────────────────────────────────────────────────────────────

void log4c_init() {
    _applyJson(DEFAULT_CONFIG);
    _loadFromFilesystem();
    _applyCompileTimeOverrides();
    _start();
}

void log4c_init(const char* configJson) {
    _applyJson(DEFAULT_CONFIG);
    if (configJson) _applyJson(configJson);
    _applyCompileTimeOverrides();
    _start();
}

void log4c_set_device(const String& name) { _device = name; }

void log4c_set_mqtt(const String& topic, Log4cPublishFn publish) {
    if (_mutex) xSemaphoreTake(_mutex, portMAX_DELAY);
    _mqttTopic   = topic;
    _mqttPublish = publish;
    _mqttEnabled = true;
    if (_mutex) xSemaphoreGive(_mutex);
}

void log4c_console_enable(bool on) { _consoleEnabled = on; }
void log4c_mqtt_enable(bool on)    { _mqttEnabled    = on; }
void log4c_console_level(int lvl)  { _consoleLevel   = lvl; }
void log4c_mqtt_level(int lvl)     { _mqttLevel      = lvl; }

// Called from main thread — always non-blocking, never prints directly
void log4c_write(int level, const char* file, int line, const char* fmt, ...) {
    char buf[220];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    const char* slash = strrchr(file, '/');
    if (!slash) slash = strrchr(file, '\\');
    const char* fname = slash ? slash + 1 : file;

    Entry e;
    e.level = level;
    e.ms    = millis();
    strncpy(e.logger,  fname, sizeof(e.logger)  - 1); e.logger [sizeof(e.logger) -1] = 0;
    snprintf(e.message, sizeof(e.message), "%s:%d %s", fname, line, buf);
    _enqueue(e);
}
