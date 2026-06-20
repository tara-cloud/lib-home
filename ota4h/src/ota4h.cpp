#include "ota4h.h"
#include <ArduinoJson.h>
#include <HTTPUpdate.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <log4c.h>

// ─── Internal state ───────────────────────────────────────────────────────────

static WiFiClient   _wifi;
static PubSubClient _mqtt(_wifi);

static String _mqttHost;
static uint16_t _mqttPort   = 1883;
static String _projectId;
static String _deviceName;
static String _topicSub;   // {projectId}.{deviceName}.ota
static String _topicPub;   // {projectId}.{deviceName}.ota_status
static String _clientId;   // {deviceName}-ota4h

static Ota4hStateFn _stateFn = nullptr;

// ─── Forward declarations ─────────────────────────────────────────────────────

static void _connect();
static void _callback(const char* topic, byte* payload, unsigned int length);
static void _handleOTA(const String& json);
static void _publishStatus(const String& state, const String& version,
                           int progress = -1, const String& error = "");

// ─── Public API ───────────────────────────────────────────────────────────────

void ota4h_init(const String& mqttHost, uint16_t mqttPort,
                const String& projectId, const String& deviceName) {
    _mqttHost   = mqttHost;
    _mqttPort   = mqttPort;
    _projectId  = projectId;
    _deviceName = deviceName;

    _topicSub = projectId + "." + deviceName + ".ota";
    _topicPub = projectId + "." + deviceName + ".ota_status";
    _clientId = deviceName + "-ota4h";

    _mqtt.setServer(_mqttHost.c_str(), _mqttPort);
    _mqtt.setCallback(_callback);

    _connect();
}

void ota4h_loop() {
    if (!_mqtt.connected()) {
        LWARN("ota4h: MQTT disconnected — reconnecting");
        _connect();
    }
    _mqtt.loop();
}

void ota4h_on_state(Ota4hStateFn fn) {
    _stateFn = fn;
}

bool ota4h_publish(const char* topic, const char* payload) {
    return _mqtt.publish(topic, payload, false);
}

// ─── Internal: connect ────────────────────────────────────────────────────────

static void _connect() {
    if (_mqttHost.length() == 0) {
        LWARN("ota4h: no mqttHost — skipping connect");
        return;
    }

    LINFO("ota4h: connecting to %s:%d", _mqttHost.c_str(), _mqttPort);

    int attempts = 0;
    while (!_mqtt.connected() && attempts++ < 5) {
        if (_mqtt.connect(_clientId.c_str())) {
            _mqtt.subscribe(_topicSub.c_str(), 1);
            LINFO("ota4h: connected — subscribed to %s", _topicSub.c_str());

            // Wire log4c MQTT appender through this client
            String logTopic = _projectId + "/" + _deviceName + "/logs";
            log4c_set_mqtt(logTopic, [](const char* t, const char* p) -> bool {
                return _mqtt.publish(t, p, false);
            });
        } else {
            LWARN("ota4h: connect failed (state=%d), retrying", _mqtt.state());
            delay(2000);
        }
    }

    if (!_mqtt.connected()) {
        LERROR("ota4h: could not connect after %d attempts", attempts - 1);
    }
}

// ─── Internal: MQTT callback ──────────────────────────────────────────────────

static void _callback(const char* topic, byte* payload, unsigned int length) {
    String msg = String((char*)payload, length);
    LDEBUG("ota4h: recv topic=%s msg=%s", topic, msg.c_str());
    _handleOTA(msg);
}

// ─── Internal: status publisher ───────────────────────────────────────────────

static void _publishStatus(const String& state, const String& version,
                           int progress, const String& error) {
    JsonDocument doc;
    doc["state"]   = state;
    doc["version"] = version;
    if (progress >= 0)      doc["progress"] = progress;
    if (error.length() > 0) doc["error"]    = error;

    String msg;
    serializeJson(doc, msg);
    _mqtt.publish(_topicPub.c_str(), msg.c_str(), false);
    LINFO("ota4h: status → %s", msg.c_str());

    if (_stateFn) {
        _stateFn(state, progress);
    }
}

// ─── Internal: OTA handler ────────────────────────────────────────────────────

static void _handleOTA(const String& json) {
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) {
        LERROR("ota4h: JSON parse failed");
        return;
    }

    String version = doc["version"] | String("");
    String url     = doc["url"]     | String("");
    String apiKey  = doc["apiKey"]  | String("");

    if (url.length() == 0) {
        LERROR("ota4h: no url in payload");
        _publishStatus("failed", version, -1, "no url");
        return;
    }

    LINFO("ota4h: starting OTA v%s from %s", version.c_str(), url.c_str());
    _publishStatus("downloading", version);

    WiFiClient wifiCli;
    HTTPClient http;
    http.begin(wifiCli, url);
    if (apiKey.length() > 0) {
        http.addHeader("x-pocket-token", apiKey);
    }

    httpUpdate.onProgress([&version](int recv, int total) {
        if (total <= 0) return;
        int pct = (recv * 100) / total;
        static int lastPct = -1;
        if (pct != lastPct && pct % 10 == 0) {
            lastPct = pct;
            LDEBUG("ota4h: progress %d%%", pct);
            _publishStatus("progress", version, pct);
        }
    });

    t_httpUpdate_return ret = httpUpdate.update(http);

    switch (ret) {
        case HTTP_UPDATE_FAILED: {
            String err = httpUpdate.getLastErrorString();
            LERROR("ota4h: failed — %s", err.c_str());
            _publishStatus("failed", version, -1, err);
            break;
        }
        case HTTP_UPDATE_NO_UPDATES:
            LINFO("ota4h: no update needed");
            _publishStatus("ok", version, 100);
            break;

        case HTTP_UPDATE_OK:
            LINFO("ota4h: success — rebooting");
            _publishStatus("ok", version, 100);
            // HTTPUpdate reboots automatically on HTTP_UPDATE_OK
            break;
    }
}
