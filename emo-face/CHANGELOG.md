# Changelog — emo-face

## [1.0.0] — 2026-06-21

### Added

- Initial release: TaraExpressions implementation of the face.h interface
- `emo_face_init(IDisplay*)` — accepts any IDisplay adapter; constructs TaraExpressions in-place (no heap)
- `renderIdleFace()` — animated idle via `TaraExpressions::animateIdle()` (random blink + pupil drift)
- `renderConfusedFace()` — asymmetric brows, squinted right eye, wavy mouth, question mark
- Ships `IDisplay.h`, `TaraExpressions.h/.cpp`, `U8g2Display.h`, `SSD1306Display.h`
- Swappable: replace emo-face in lib_deps with any other face.h implementation — no other code changes
