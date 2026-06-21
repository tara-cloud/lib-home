#pragma once
#include <face.h>
#include <IDisplay.h>
#include <functional>

// ─── tara-face — IDisplay-backed FaceRenderer builder ────────────────────────
//
// Provides a configurable FaceRenderer that is wired to an IDisplay.
// Callers supply per-state render functions (lambdas or free functions).
// No pixel drawing is done here — all rendering is the caller's responsibility.
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   #include <tara_face.h>
//
//   TaraFace face(&myDisplay);
//
//   face.on(FACE_IDLE, []() {
//       // draw idle face using myDisplay primitives
//   });
//   face.on(FACE_ERROR, []() {
//       // draw error face
//   });
//
//   face.begin();   // calls face_register() with the built FaceRenderer
//
//   // In loop():
//   renderFace(toFaceState(currentState));
//
// ─── Fallback ─────────────────────────────────────────────────────────────────
// States with no registered handler are silently skipped (nullptr slot).
// Call setFallback() to draw something for any unregistered state.

using TaraFaceRenderFn = std::function<void()>;

class TaraFace {
public:
    explicit TaraFace(IDisplay* display);

    // Register a render function for a specific state.
    // Replaces any previous registration for that state.
    TaraFace& on(FaceState state, TaraFaceRenderFn fn);

    // Register a fallback called for any state with no specific handler.
    TaraFace& setFallback(TaraFaceRenderFn fn);

    // Build the FaceRenderer from registered handlers and call face_register().
    // Call once in setup() after all on() registrations.
    void begin();

    // Access the underlying display for drawing inside render functions.
    IDisplay* display() const;

private:
    IDisplay*       _display;
    TaraFaceRenderFn _handlers[FACE_STATE_COUNT];
    TaraFaceRenderFn _fallback;
};
