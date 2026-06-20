#pragma once
#include <Arduino.h>

// ─── ota4h — OTA-over-MQTT library ───────────────────────────────────────────
//
// Manages firmware updates triggered via an MQTT message.
//
// Inbound topic  : {projectId}.{deviceName}.ota
// Inbound payload: { "version": "1.2.3", "url": "http://...", "apiKey": "..." }
//
// Outbound topic : {projectId}.{deviceName}.ota_status
// Outbound payload: { "state": "downloading|progress|ok|failed",
//                     "version": "...", "progress": 0-100, "error": "..." }
//
// Usage:
//   1. Call ota4h_init() once after WiFi + broker info is available.
//   2. Call ota4h_loop() in every loop() iteration.
//   3. Optionally register ota4h_on_state() to hook state changes into your UI.

// ─── Optional state-change hook ──────────────────────────────────────────────
// Called when OTA state transitions (downloading, progress, ok, failed).
// progress is -1 when not applicable.
using Ota4hStateFn = std::function<void(const String& state, int progress)>;

// ─── Public API ──────────────────────────────────────────────────────────────

// Call once after WiFi is up and broker credentials are known.
// Registers own dedicated MQTT client; does NOT share the main client.
// mqttHost / mqttPort / projectId / deviceName — used to derive topics and clientId.
void ota4h_init(const String& mqttHost, uint16_t mqttPort,
                const String& projectId, const String& deviceName);

// Call every loop() — keeps MQTT connection alive and dispatches inbound messages.
void ota4h_loop();

// Optional: register a callback that fires on every OTA state change.
void ota4h_on_state(Ota4hStateFn fn);

// ─── Internal ────────────────────────────────────────────────────────────────
// Exposed so log4c MQTT appender wiring can be done externally if desired.
// Returns a reference to the internal publish function bound to the OTA client.
bool ota4h_publish(const char* topic, const char* payload);
