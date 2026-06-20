#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>

// ─── config4h — device configuration over MQTT ───────────────────────────────
//
// Subscribes to {projectId}.{deviceName}.config, keeps the latest JSON payload
// in a static JsonDocument, and exposes dot-path access to individual values.
//
// ─── MQTT message (inbound) ──────────────────────────────────────────────────
// Topic  : {projectId}.{deviceName}.config
// Payload: any flat or nested JSON object, e.g.
//   {
//     "projectId":   "tara01",
//     "projectName": "Tara Home",
//     "deviceName":  "Tara",
//     "deviceType":  "robot",
//     "healthcheck": { "enabled": true, "frequency": 100 }
//   }
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   config4h_init("192.168.1.10", 1883, "tara01", "Tara");
//
//   // in loop():
//   config4h_loop();
//
//   // read values anywhere, any time:
//   bool   en  = config4h_get("healthcheck.enabled").asBool();
//   int    hz  = config4h_get("healthcheck.frequency").asInt();
//   String dev = config4h_get("deviceName").asString();
//
// ─── ConfigValue ─────────────────────────────────────────────────────────────
// Thin wrapper around a JsonVariant. Provides typed accessors and a
// .isNull() guard for missing keys.
struct ConfigValue {
    explicit ConfigValue(JsonVariant v) : _v(v) {}

    bool      asBool(bool   def = false)     const { return _v.isNull() ? def : _v.as<bool>();   }
    int       asInt(int     def = 0)         const { return _v.isNull() ? def : _v.as<int>();    }
    float     asFloat(float def = 0.0f)      const { return _v.isNull() ? def : _v.as<float>();  }
    String    asString(const String& def = "") const {
        if (_v.isNull()) return def;
        String s; serializeJson(_v, s);
        // strip surrounding quotes for plain strings
        if (s.length() >= 2 && s[0] == '"' && s[s.length()-1] == '"')
            return s.substring(1, s.length()-1);
        return s;
    }
    bool      isNull()  const { return _v.isNull(); }
    JsonVariant raw()   const { return _v; }

private:
    JsonVariant _v;
};

// ─── Change callback ─────────────────────────────────────────────────────────
using Config4hChangeFn = std::function<void()>;

// ─── Init ─────────────────────────────────────────────────────────────────────

// Connect dedicated MQTT client and subscribe to config topic.
// Call once after WiFi is up and broker details are known.
void config4h_init(const String& mqttHost, uint16_t mqttPort,
                   const String& projectId, const String& deviceName);

// Call every loop() — keeps MQTT alive and dispatches inbound messages.
void config4h_loop();

// Optional: called every time a new config payload arrives.
void config4h_on_change(Config4hChangeFn fn);

// ─── Read ─────────────────────────────────────────────────────────────────────

// Read a value by dot-path key (e.g. "healthcheck.enabled", "deviceName").
// Returns ConfigValue wrapping a null variant if the key doesn't exist.
ConfigValue config4h_get(const char* path);

// Returns true if at least one config payload has been received.
bool config4h_has_config();

// Returns the raw stored JSON as a String (useful for debugging).
String config4h_raw();
