# Changelog — tara-face

## [1.0.0] — 2026-06-21

### Added

- Initial release: IDisplay-backed FaceRenderer builder
- `TaraFace(IDisplay*)` — construct with any IDisplay adapter
- `TaraFace::on(FaceState, fn)` — register a render function per state (chainable)
- `TaraFace::setFallback(fn)` — fallback for states with no specific handler (chainable)
- `TaraFace::begin()` — build FaceRenderer and call `face_register()`
- `TaraFace::display()` — access the IDisplay for use inside render lambdas
- No pixel drawing — all rendering is the caller's responsibility
