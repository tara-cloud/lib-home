#pragma once
#include <Arduino.h>
#include <functional>

// ─── health-check — periodic health-check over MQTT ──────────────────────────
//
// Reads healthcheck config from config4h and publishes a health-check packet
// to MQTT at the configured frequency. Reads component list from reg4h to
// populate the components array in the payload.
//
// Requires config4h to be initialised first — reads:
//   healthcheck.enabled   (bool)  — master switch
//   healthcheck.frequency (int)   — interval in seconds between publishes
//
// ─── Outbound topic ──────────────────────────────────────────────────────────
// Topic  : {projectId}.{deviceName}.healthcheck
// Payload:
//   {
//     "projectId":  "tara01",
//     "deviceName": "Tara",
//     "timestamp":  "2026-06-11 12:12:13.345 IST",
//     "status":     "Healthy",
//     "components": [
//       { "name": "OLED",        "status": "Healthy" },
//       { "name": "TouchSensor", "status": "Healthy" }
//     ]
//   }
//
// Component status is "Healthy" by default. Call health_check_set_status()
// to mark a specific component as degraded or unhealthy.
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   // After config4h_init() and reg4h_add_component() calls:
//   health_check_init("192.168.1.10", 1883, "tara01", "Tara");
//
//   // In loop():
//   health_check_loop();    // publishes when interval elapses and enabled=true
//
//   // Optionally mark a component unhealthy:
//   health_check_set_status("OLED", "Unhealthy");

// ─── Component status override ────────────────────────────────────────────────
// Sets the reported status string for a named component.
// Name match is case-sensitive and must match reg4h component name exactly.
// Default status for all components is "Healthy".
void health_check_set_status(const String& componentName, const String& status);

// ─── Init ─────────────────────────────────────────────────────────────────────
// Connects a dedicated MQTT client for publishing health-check packets.
// Call once after WiFi is up, config4h is initialised, and reg4h components
// are registered.
void health_check_init(const String& mqttHost, uint16_t mqttPort,
                       const String& projectId, const String& deviceName);

// ─── Loop ─────────────────────────────────────────────────────────────────────
// Call every loop() iteration.
// Reads healthcheck.enabled and healthcheck.frequency from config4h on each
// tick so changes take effect immediately without restart.
// Publishes when: enabled == true && millis() - lastPublish >= frequency * 1000
void health_check_loop();

// ─── Manual publish ───────────────────────────────────────────────────────────
// Publish a health-check packet immediately, regardless of frequency or enabled.
void health_check_publish();
