# lib-home

Shared ESP32/Arduino libraries hosted on [Pocket](http://192.168.0.107:30600).  
Each library is a standalone PlatformIO package — just add one line to `lib_deps`.

---

## How to use a library

Add the Pocket download URL to `lib_deps` in your `platformio.ini`:

```ini
lib_deps =
    http://192.168.0.107:30600/api/files/<repo>/<artifact>/<version>/<artifact>.zip?token=<api-key>
```

PlatformIO downloads and unpacks it automatically — no local path needed.

---

## Libraries

### log4c `1.0.0`

Structured logging for ESP32 — prints to Serial immediately, publishes to MQTT asynchronously via a FreeRTOS drain task.

**Install:**

```ini
lib_deps =
    http://192.168.0.107:30600/api/files/log4c/log4c/1.0.0/log4c.zip?token=<api-key>
    bblanchon/ArduinoJson@^7.0.0
```

**Usage:**

```cpp
#include <log4c.h>

void setup() {
    Serial.begin(115200);

    // Serial only — works immediately
    log4c_init();

    // With MQTT (set after your MQTT client connects)
    log4c_set_device("MyDevice");
}

// Call once MQTT is connected:
void onMqttConnected() {
    log4c_set_mqtt({
        "myproject/log",
        [](const char* topic, const char* payload) {
            return mqttClient.publish(topic, payload);
        }
    });
}

void loop() {
    LDEBUG("sensor value: %d", analogRead(34));
    LINFO("loop tick");
    LWARN("low memory: %d bytes", ESP.getFreeHeap());
    LERROR("connection failed");
}
```

**MQTT payload:**

```json
{
  "level":     "INFO",
  "logger":    "main.cpp",
  "message":   "loop tick",
  "timestamp": 12345,
  "device":    "MyDevice"
}
```

**Tuning:**

```cpp
Log4cConfig cfg;
cfg.serialLevel  = L4C_DEBUG;  // print everything to Serial
cfg.mqttLevel    = L4C_INFO;   // send INFO and above over MQTT
cfg.queueSize    = 64;         // larger buffer for bursty logging
cfg.taskCore     = 0;          // drain task core (0 = background)
log4c_init(cfg);
```

---

### ota4h `1.0.0`

OTA firmware update over MQTT for ESP32 — dedicated MQTT client, progress reporting, log4c integration.

**Install:**

```ini
lib_deps =
    http://192.168.0.107:30600/api/files/ota4h/ota4h/1.0.0/ota4h.zip?token=<api-key>
    bblanchon/ArduinoJson@^7.0.0
    knolleary/PubSubClient@^2.8
```

**Usage:**

```cpp
#include <ota4h.h>

void setup() {
    // ... WiFi connect ...
    ota4h_on_state([](const String& state, int pct) {
        // update display or LED based on state
    });
    ota4h_init("192.168.1.10", 1883, "myproject", "device01");
}

void loop() {
    ota4h_loop();
}
```

**MQTT trigger payload** (publish to `{projectId}.{deviceName}.ota`):

```json
{ "version": "1.2.3", "url": "http://...", "apiKey": "..." }
```

**Status payload** (published to `{projectId}.{deviceName}.ota_status`):

```json
{ "state": "downloading|progress|ok|failed", "version": "1.2.3", "progress": 50, "error": "..." }
```

---

### wifi4h `1.0.0`

WiFi connection + captive-portal hotspot for ESP32 — consumer-defined field schema, NVS persistence, schema-driven form rendering.

**Install:**

```ini
lib_deps =
    http://192.168.0.107:30600/api/files/wifi4h/wifi4h/1.0.0/wifi4h.zip?token=<api-key>
    bblanchon/ArduinoJson@^7.0.0
```

**Usage:**

```cpp
#include <wifi4h.h>

void setup() {
    wifi4h_set_device_id("A1B2C3");               // suffix for AP name "Setup-A1B2C3"

    // Declare fields — portal renders a form with exactly these fields
    wifi4h_add_field("ssid",        "string",   true);
    wifi4h_add_field("password",    "password", false);
    wifi4h_add_field("host",        "string",   true);
    wifi4h_add_field("mqttPort",    "number",   false);

    wifi4h_on_event([](const String& ev, const String& detail) {
        Serial.printf("[wifi] %s %s\n", ev.c_str(), detail.c_str());
    });

    wifi4h_load();       // restore saved values from NVS
    wifi4h_connect();    // connect to saved SSID, or start captive portal

    // Read any saved value by field name
    String host     = wifi4h_get("host");
    String mqttPort = wifi4h_get("mqttPort");
}

void loop() {
    wifi4h_reconnect();  // reconnect silently if WiFi drops
}
```

**Events:** `connected` (detail = IP), `failed` (detail = SSID), `ap_start` (detail = AP name + IP), `saved`

**Portal behaviour:**

- AP name: `Setup-<last 6 chars of device ID>`, password `12345678`
- Captive portal probes handled: iOS, Android, Windows
- POST `/save` validates required fields, persists all to NVS, reboots

---

### config4h `1.0.0`

Device configuration over MQTT for ESP32 — single JSON document in memory, dot-path access, live change callbacks.

**Install:**

```ini
lib_deps =
    http://192.168.0.107:30600/api/files/config4h/config4h/1.0.0/config4h.zip?token=<api-key>
    bblanchon/ArduinoJson@^7.0.0
    knolleary/PubSubClient@^2.8
```

**Usage:**

```cpp
#include <config4h.h>

void setup() {
    config4h_on_change([]() {
        bool en  = config4h_get("healthcheck.enabled").asBool();
        int  hz  = config4h_get("healthcheck.frequency").asInt();
        String n = config4h_get("deviceName").asString();
    });
    config4h_init("192.168.1.10", 1883, "tara01", "Tara");
}

void loop() {
    config4h_loop();

    // Read anywhere, any time — returns default if key missing or no config yet
    if (config4h_get("healthcheck.enabled").asBool()) { ... }
}
```

**MQTT trigger** (publish to `{projectId}.{deviceName}.config`):

```json
{
  "projectId":   "tara01",
  "projectName": "Tara Home",
  "deviceName":  "Tara",
  "deviceType":  "robot",
  "healthcheck": { "enabled": true, "frequency": 100 }
}
```

---

### reg4h `1.0.0`

Component registry for ESP32 — unified I2C scan + GPIO registration. One call for all hardware; read-back accessors for registration payloads.

**Install:**

```ini
lib_deps =
    http://192.168.0.107:30600/api/files/reg4h/reg4h/1.0.0/reg4h.zip?token=<api-key>
```

**Usage:**

```cpp
#include <reg4h.h>

void setup() {
    // I2C — scans bus, adds one entry per found address (OLED, MPU6050, etc.)
    uint8_t i2cPins[] = {21, 22};
    reg4h_add_component("", "", "I2C", i2cPins, 2);

    // GPIO / other protocols
    uint8_t touchPins[] = {18};
    reg4h_add_component("TouchSensor", "input", "GPIO", touchPins, 1);
    // → stored as pin="GPIO18", label="PIN18", direction="input"

    // Read back for registration payload
    for (int i = 0; i < reg4h_component_count(); i++) {
        const Reg4hComponent* c = reg4h_get_component(i);
        Serial.println(c->name);
    }
}
```

---

## Publishing a library update

```bash
cd lib-home
zip -r /tmp/<lib>.zip <lib>/
curl -X POST http://192.168.0.107:30600/api/repos/<lib>/artifacts \
  -H "x-pocket-token: <api-key>" \
  -F "name=<lib>" -F "version=<new-version>" \
  -F "file=@/tmp/<lib>.zip;filename=<lib>.zip"
```
