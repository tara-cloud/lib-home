#include "health_check.h"
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <log4c.h>
#include <config4h.h>
#include <reg4h.h>

// ─── Internal state ───────────────────────────────────────────────────────────

static WiFiClient   _wifi;
static PubSubClient _mqtt(_wifi);

static String   _mqttHost;
static uint16_t _mqttPort   = 1883;
static String   _projectId;
static String   _deviceName;
static String   _firmwareVersion;
static String   _topic;
static String   _clientId;

static unsigned long _lastPublish = 0;

// Per-component status overrides — parallel array to reg4h component list.
// Indexed by component name. Max 8 entries (matches REG4H_MAX_COMPONENTS).
static const int _MAX_OVERRIDES = 8;
struct _StatusOverride { String name; String status; };
static _StatusOverride _overrides[_MAX_OVERRIDES];
static int             _overrideCount = 0;

// ─── Helpers ──────────────────────────────────────────────────────────────────

static void _connect() {
    if (_mqttHost.length() == 0) {
        LWARN("health-check: no mqttHost — skipping");
        return;
    }
    if (_mqtt.connected()) return;

    LINFO("health-check: connecting to %s:%d", _mqttHost.c_str(), _mqttPort);
    int attempts = 0;
    while (!_mqtt.connected() && attempts++ < 5) {
        if (_mqtt.connect(_clientId.c_str())) {
            LINFO("health-check: MQTT connected");
        } else {
            LWARN("health-check: connect failed (state=%d), retrying", _mqtt.state());
            delay(2000);
        }
    }
    if (!_mqtt.connected()) {
        LERROR("health-check: could not connect after %d attempts", attempts - 1);
    }
}

// Returns the status for a component name — override if set, else "Healthy".
static String _statusFor(const String& name) {
    for (int i = 0; i < _overrideCount; i++) {
        if (_overrides[i].name == name) return _overrides[i].status;
    }
    return HC_HEALTHY;
}

// Build ISO-like timestamp from system time.
// Format: "YYYY-MM-DD HH:MM:SS.mmm IST"
// Falls back to millis-based string if time is not synchronised (year < 2020).
static String _timestamp() {
    struct tm ti;
    if (getLocalTime(&ti) && ti.tm_year + 1900 >= 2020) {
        char buf[40];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.000 IST",
            ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
            ti.tm_hour, ti.tm_min, ti.tm_sec);
        return String(buf);
    }
    // Fallback — no NTP sync yet
    unsigned long ms = millis();
    char buf[32];
    snprintf(buf, sizeof(buf), "uptime:%lums", ms);
    return String(buf);
}

// ─── Public API ───────────────────────────────────────────────────────────────

void health_check_set_status(const String& componentName, const String& status) {
    for (int i = 0; i < _overrideCount; i++) {
        if (_overrides[i].name == componentName) {
            _overrides[i].status = status;
            return;
        }
    }
    if (_overrideCount < _MAX_OVERRIDES) {
        _overrides[_overrideCount++] = { componentName, status };
    }
}

void health_check_init(const String& mqttHost, uint16_t mqttPort,
                       const String& projectId, const String& deviceName,
                       const String& firmwareVersion) {
    _mqttHost        = mqttHost;
    _mqttPort        = mqttPort;
    _projectId       = projectId;
    _deviceName      = deviceName;
    _firmwareVersion = firmwareVersion;
    _topic      = projectId + "." + deviceName + ".healthcheck";
    _clientId   = deviceName + "-hc";

    _mqtt.setServer(_mqttHost.c_str(), _mqttPort);
    _connect();
}

void health_check_loop() {
    if (!_mqtt.connected()) _connect();
    _mqtt.loop();

    bool enabled = config4h_get("healthcheck.enabled").asBool(false);
    if (!enabled) return;

    int freqSec = config4h_get("healthcheck.frequency").asInt(60);
    if (freqSec <= 0) freqSec = 60;

    if (millis() - _lastPublish >= (unsigned long)freqSec * 1000UL) {
        health_check_publish();
    }
}

void health_check_publish() {
    if (!_mqtt.connected()) {
        _connect();
        if (!_mqtt.connected()) {
            LWARN("health-check: publish skipped — no MQTT connection");
            return;
        }
    }

    JsonDocument doc;
    doc["projectId"]       = _projectId;
    doc["deviceName"]      = _deviceName;
    doc["firmwareVersion"] = _firmwareVersion;
    doc["timestamp"]       = _timestamp();
    doc["status"]          = HC_ONLINE;

    JsonArray comps = doc["components"].to<JsonArray>();
    for (int i = 0; i < reg4h_component_count(); i++) {
        const Reg4hComponent* c = reg4h_get_component(i);
        JsonObject co = comps.add<JsonObject>();
        co["name"]   = c->name;
        co["status"] = _statusFor(c->name);
    }

    String payload;
    serializeJson(doc, payload);

    if (_mqtt.publish(_topic.c_str(), payload.c_str(), false)) {
        LINFO("health-check: published to %s", _topic.c_str());
    } else {
        LWARN("health-check: publish failed (buffer overflow or disconnected)");
    }

    _lastPublish = millis();
}
