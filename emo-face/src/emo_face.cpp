#include "emo_face.h"
#include "TaraExpressions.h"

// ─── Internal state ───────────────────────────────────────────────────────────

static IDisplay*       _display    = nullptr;
static TaraExpressions* _expr      = nullptr;

// TaraExpressions lives in static storage — no heap allocation.
// Aligned storage avoids constructing before emo_face_init() is called.
static alignas(TaraExpressions) uint8_t _exprStorage[sizeof(TaraExpressions)];

// ─── Init ─────────────────────────────────────────────────────────────────────

void emo_face_init(IDisplay* display) {
    _display = display;
    _expr    = new (_exprStorage) TaraExpressions(display);
}

// ─── face.h contract implementations ─────────────────────────────────────────

void renderIdleFace() {
    if (_expr) _expr->animateIdle();
}

void renderConfusedFace() {
    if (!_display) return;
    _display->clear();

    // Asymmetric brows — left raised, right furrowed
    _display->drawHLine(14, 10, 16, true);
    _display->drawHLine(14, 11, 16, true);
    // Right brow — angled down-to-up (inner end lower)
    for (int i = 0; i < 16; i++)
        _display->drawPixel(66 + i, 13 - (i * 4) / 16, true);

    // Left eye — round, open
    _display->drawCircle(22, 28, 9, true);
    _display->fillCircle(22, 28, 4, true);
    // Right eye — squinted (smaller, lower)
    _display->fillCircle(74, 32, 6, true);

    // Wavy confused mouth
    _display->drawHLine(36, 50, 8, true);
    _display->drawHLine(44, 46, 8, true);
    _display->drawHLine(52, 50, 8, true);
    _display->drawHLine(60, 46, 8, true);

    // Question mark top-right
    _display->setTextSize(1);
    _display->setTextColor(true);
    _display->setCursor(104, 5);
    _display->print("?");

    _display->show();
}
