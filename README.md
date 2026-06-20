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

## Publishing a library update

```bash
cd lib-home
zip -r /tmp/<lib>.zip <lib>/
curl -X POST http://192.168.0.107:30600/api/repos/<lib>/artifacts \
  -H "x-pocket-token: <api-key>" \
  -F "name=<lib>" -F "version=<new-version>" \
  -F "file=@/tmp/<lib>.zip;filename=<lib>.zip"
```
