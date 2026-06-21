#include "reg4h.h"
#include <Wire.h>
#include <log4c.h>

// ─── KNOWN_I2C table ──────────────────────────────────────────────────────────

struct KnownI2CDevice { uint8_t addr; const char* name; const char* type; };

static const KnownI2CDevice KNOWN_I2C[] = {
    { 0x3C, "OLED",    "output" },
    { 0x3D, "OLED",    "output" },
    { 0x68, "MPU6050", "io"     },
    { 0x69, "MPU6050", "io"     },
    { 0x76, "BME280",  "input"  },
    { 0x77, "BME280",  "input"  },
    { 0x40, "INA219",  "input"  },
    { 0x27, "LCD",     "output" },
    { 0x3F, "LCD",     "output" },
};
static const int KNOWN_I2C_COUNT = sizeof(KNOWN_I2C) / sizeof(KNOWN_I2C[0]);

// ─── Internal state ───────────────────────────────────────────────────────────

static Reg4hComponent _components[REG4H_MAX_COMPONENTS];
static int            _componentCount = 0;

// ─── Helpers ──────────────────────────────────────────────────────────────────

static const char* _dirFromType(const String& type) {
    if (type == "input")  return "input";
    if (type == "output") return "output";
    return "io";
}

// ─── Public API ───────────────────────────────────────────────────────────────

void reg4h_add_component(const String& name, const String& type,
                         const String& protocol,
                         const uint8_t* pins, int pinCount) {
    if (protocol == "I2C") {
        if (pinCount < 2) {
            LWARN("reg4h: I2C requires 2 pins (SDA, SCL) — skipping");
            return;
        }
        int sda = pins[0];
        int scl = pins[1];

        Wire.begin(sda, scl);
        LINFO("reg4h: I2C scan SDA=%d SCL=%d", sda, scl);

        for (uint8_t addr = 1; addr < 127 && _componentCount < REG4H_MAX_COMPONENTS; addr++) {
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() != 0) continue;

            LDEBUG("reg4h: I2C found 0x%02X", addr);

            Reg4hComponent& c = _components[_componentCount++];
            c.protocol = "I2C";
            c.address  = addr;
            c.pinCount = 2;
            c.pins[0]  = { "GPIO" + String(sda), "SDA", "io"     };
            c.pins[1]  = { "GPIO" + String(scl), "SCL", "output" };

            bool matched = false;
            for (int k = 0; k < KNOWN_I2C_COUNT; k++) {
                if (KNOWN_I2C[k].addr == addr) {
                    c.name    = KNOWN_I2C[k].name;
                    c.type    = KNOWN_I2C[k].type;
                    matched   = true;
                    break;
                }
            }
            if (!matched) {
                char buf[16];
                snprintf(buf, sizeof(buf), "Unknown(0x%02X)", addr);
                c.name = name.length() > 0 ? name : String(buf);
                c.type = type.length() > 0 ? type : String("io");
            }

            LINFO("reg4h: component %s @ 0x%02X", c.name.c_str(), addr);
        }

        Wire.end();
        LINFO("reg4h: scan done — %d component(s) total", _componentCount);

    } else {
        if (_componentCount >= REG4H_MAX_COMPONENTS) {
            LWARN("reg4h: max components reached — skipping %s", name.c_str());
            return;
        }

        Reg4hComponent& c = _components[_componentCount++];
        c.name     = name;
        c.type     = type;
        c.protocol = protocol;
        c.address  = 0;
        c.pinCount = 0;

        const char* dir = _dirFromType(type);
        for (int i = 0; i < pinCount && c.pinCount < REG4H_MAX_PINS_PER_COMP; i++) {
            char label[12];
            snprintf(label, sizeof(label), "PIN%d", pins[i]);
            c.pins[c.pinCount++] = { "GPIO" + String(pins[i]), String(label), String(dir) };
        }

        LINFO("reg4h: added %s (%s/%s) pins=%d", name.c_str(), type.c_str(), protocol.c_str(), c.pinCount);
    }
}

int reg4h_component_count() {
    return _componentCount;
}

const Reg4hComponent* reg4h_get_component(int index) {
    if (index < 0 || index >= _componentCount) return nullptr;
    return &_components[index];
}
