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
// ─── Status values ────────────────────────────────────────────────────────────
// HC_ONLINE   "Online"   — device/component is reachable and functioning
// HC_HEALTHY  "Healthy"  — all checks pass (default for components)
// HC_ERROR    "Error"    — reachable but in a fault state
// HC_OFFLINE  "Offline"  — unreachable or powered off
//
// ─── Outbound topic ──────────────────────────────────────────────────────────
// Topic  : {projectId}.{deviceName}.healthcheck
// Payload:
//   {
//     "projectId":  "tara01",
//     "deviceName": "Tara",
//     "timestamp":  "2026-06-11 12:12:13.000 IST",
//     "status":     "Online",
//     "components": [
//       { "name": "OLED",        "status": "Healthy" },
//       { "name": "TouchSensor", "status": "Healthy" }
//     ]
//   }
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   health_check_init("192.168.1.10", 1883, "tara01", "Tara");
//
//   // In loop():
//   health_check_loop();
//
//   // Mark a component degraded:
//   health_check_set_status("OLED", HC_ERROR);

// ─── Status constants ─────────────────────────────────────────────────────────
static const char* HC_ONLINE  = "Online";
static const char* HC_HEALTHY = "Healthy";
static const char* HC_ERROR   = "Error";
static const char* HC_OFFLINE = "Offline";

// ─── Component status override ────────────────────────────────────────────────
// Sets the reported status for a named component.
// Name match is case-sensitive; must match reg4h component name exactly.
// Default for all components is HC_HEALTHY.
void health_check_set_status(const String& componentName, const String& status);

// ─── Init ─────────────────────────────────────────────────────────────────────
void health_check_init(const String& mqttHost, uint16_t mqttPort,
                       const String& projectId, const String& deviceName,
                       const String& firmwareVersion);

// ─── Loop ─────────────────────────────────────────────────────────────────────
// Call every loop(). Publishes when enabled=true and frequency elapses.
void health_check_loop();

// ─── Manual publish ───────────────────────────────────────────────────────────
// Publish immediately regardless of frequency or enabled flag.
void health_check_publish();
