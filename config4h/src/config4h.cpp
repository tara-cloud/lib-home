#include "config4h.h"
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <log4c.h>

// ─── Internal state ───────────────────────────────────────────────────────────

static WiFiClient   _wifi;
static PubSubClient _mqtt(_wifi);

static String           _mqttHost;
static uint16_t         _mqttPort   = 1883;
static String           _projectId;
static String           _deviceName;
static String           _clientId;
static String           _topic;

// Config document lives in static memory — never freed.
// 2 kB covers all realistic device config payloads.
static JsonDocument     _doc;
static bool             _hasConfig  = false;

static Config4hChangeFn _changeFn   = nullptr;

// ─── Forward declarations ─────────────────────────────────────────────────────

static void _connect();
static void _callback(const char* topic, byte* payload, unsigned int length);

// ─── Public API ───────────────────────────────────────────────────────────────

void config4h_init(const String& mqttHost, uint16_t mqttPort,
                   const String& projectId, const String& deviceName) {
    _mqttHost   = mqttHost;
    _mqttPort   = mqttPort;
    _projectId  = projectId;
    _deviceName = deviceName;
    _topic      = projectId + "." + deviceName + ".config";
    _clientId   = deviceName + "-cfg4h";

    _mqtt.setServer(_mqttHost.c_str(), _mqttPort);
    _mqtt.setCallback(_callback);
    _connect();
}

void config4h_loop() {
    if (!_mqtt.connected()) {
        LWARN("config4h: MQTT disconnected — reconnecting");
        _connect();
    }
    _mqtt.loop();
}

void config4h_on_change(Config4hChangeFn fn) {
    _changeFn = fn;
}

bool config4h_has_config() {
    return _hasConfig;
}

String config4h_raw() {
    if (!_hasConfig) return "{}";
    String out;
    serializeJson(_doc, out);
    return out;
}

// ─── Dot-path resolver ────────────────────────────────────────────────────────
// Walks "a.b.c" through the JsonDocument, returning the matching JsonVariant.
// Returns a null variant if any segment along the path is missing.

ConfigValue config4h_get(const char* path) {
    if (!_hasConfig) return ConfigValue(JsonVariant());

    JsonVariant cur = _doc.as<JsonVariant>();
    const char* p   = path;

    while (*p) {
        // Find next segment
        const char* dot = strchr(p, '.');
        size_t len = dot ? (size_t)(dot - p) : strlen(p);

        // Extract segment into a small buffer
        char key[64];
        if (len == 0 || len >= sizeof(key)) return ConfigValue(JsonVariant());
        memcpy(key, p, len);
        key[len] = '\0';

        if (!cur.is<JsonObject>()) return ConfigValue(JsonVariant());
        cur = cur[key];

        p = dot ? dot + 1 : p + len;
    }

    return ConfigValue(cur);
}

// ─── Internal: connect ────────────────────────────────────────────────────────

static void _connect() {
    if (_mqttHost.length() == 0) {
        LWARN("config4h: no mqttHost — skipping");
        return;
    }

    LINFO("config4h: connecting to %s:%d", _mqttHost.c_str(), _mqttPort);

    int attempts = 0;
    while (!_mqtt.connected() && attempts++ < 5) {
        if (_mqtt.connect(_clientId.c_str())) {
            _mqtt.subscribe(_topic.c_str(), 1);
            LINFO("config4h: connected — subscribed to %s", _topic.c_str());
        } else {
            LWARN("config4h: connect failed (state=%d), retrying", _mqtt.state());
            delay(2000);
        }
    }

    if (!_mqtt.connected()) {
        LERROR("config4h: could not connect after %d attempts", attempts - 1);
    }
}

// ─── Internal: MQTT callback ──────────────────────────────────────────────────

static void _callback(const char* topic, byte* payload, unsigned int length) {
    String msg = String((char*)payload, length);
    LDEBUG("config4h: recv [%s] %s", topic, msg.c_str());

    _doc.clear();
    DeserializationError err = deserializeJson(_doc, msg);
    if (err != DeserializationError::Ok) {
        LERROR("config4h: JSON parse failed: %s", err.c_str());
        return;
    }

    _hasConfig = true;
    LINFO("config4h: config updated");

    if (_changeFn) _changeFn();
}
