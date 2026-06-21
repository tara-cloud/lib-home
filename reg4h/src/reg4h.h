#pragma once
#include <Arduino.h>

// ─── reg4h — Component registry for ESP32/Arduino ────────────────────────────
//
// Single unified API for registering all hardware components — I2C (auto-scan)
// and GPIO/SPI/UART (manual). The component list is then read by the
// registration layer (e.g. registerRobot()) to build the device payload.
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   // I2C — lib scans the bus and adds one entry per found address
//   uint8_t i2cPins[] = {21, 22};          // SDA, SCL
//   reg4h_add_component("", "", "I2C", i2cPins, 2);
//
//   // GPIO / any other protocol — stored as-is
//   uint8_t touchPins[] = {18};
//   reg4h_add_component("TouchSensor", "input", "GPIO", touchPins, 1);
//   // → pin="GPIO18", label="PIN18", direction="input" (derived from type)
//
// ─── I2C behaviour ───────────────────────────────────────────────────────────
// When protocol == "I2C":
//   pins[0] = SDA, pins[1] = SCL (must supply exactly 2 pins)
//   Scans addresses 1–126 via Wire; calls Wire.end() after scan.
//   Each found address matched against KNOWN_I2C table — recognised devices
//   use the table's name/type; unrecognised ones use the caller's name/type
//   args (or "Unknown(0xNN)" / "io" if those are also empty).
//   SDA pin → label "SDA", direction "io"
//   SCL pin → label "SCL", direction "output"
//   One call may register multiple components (one per address found).
//
// ─── Non-I2C behaviour ───────────────────────────────────────────────────────
// Registers exactly one component. For each pin:
//   pin       = "GPIO{n}"
//   label     = "PIN{n}"
//   direction = derived from component type:
//               "input"  → "input"
//               "output" → "output"
//               anything else → "io"

// ─── Types ───────────────────────────────────────────────────────────────────

struct Reg4hPin {
    String pin;        // "GPIO21"
    String label;      // "SDA", "SCL", "PIN18"
    String direction;  // "io" | "input" | "output"
};

struct Reg4hComponent {
    String   name;
    String   type;
    String   protocol;
    uint8_t  address;   // I2C address; 0 = not I2C
    Reg4hPin pins[4];
    int      pinCount;
};

static const int REG4H_MAX_COMPONENTS    = 8;
static const int REG4H_MAX_PINS_PER_COMP = 4;

// ─── API ─────────────────────────────────────────────────────────────────────

void reg4h_add_component(const String& name, const String& type,
                         const String& protocol,
                         const uint8_t* pins, int pinCount);

int                   reg4h_component_count();
const Reg4hComponent* reg4h_get_component(int index);
