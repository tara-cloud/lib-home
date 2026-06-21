# Changelog — tara-face

## [2.0.0] — 2026-06-21

### Changed (breaking)

- Complete rewrite — now a full RoboEyes-inspired renderer using IDisplay
- Replaces the bare lambda-registration builder from v1.0.0

### Added

- `TaraFace(IDisplay*, screenW, screenH, fps)` — full constructor
- `setWidth/Height/BorderRadius/SpaceBetween()` — eye shape configuration
- `setMood(TARA_DEFAULT/TIRED/ANGRY/HAPPY)` — mood with smooth eyelid tweening
- `setPosition(TARA_N/NE/E/SE/S/SW/W/NW/CENTER)` — gaze direction with smooth movement
- `setCuriosity(bool)` — outer eye enlarges when looking sideways
- `setCyclops(bool)` — single-eye mode
- `close/open/blink()` — manual blink; per-eye variants: `close(left, right)`
- `setAutoblinker(bool, intervalSec, variationSec)` — random automatic blink
- `setIdleMode(bool, intervalSec, variationSec)` — random eye repositioning
- `setHFlicker/setVFlicker(bool, amplitude)` — horizontal/vertical shiver
- `setSweat(bool)` — animated sweat drops
- `anim_confused()` / `anim_laugh()` — one-shot macro animations
- `begin()` — builds and registers `FaceRenderer` with face.h dispatcher
- All geometry animated via linear tweening `(current + target) / 2` per frame
- Frame-rate limiting via `update()` using `millis()`, no blocking delays

## [1.0.0] — 2026-06-21

### Added

- Initial release: bare lambda-registration FaceRenderer builder
