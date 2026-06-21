#pragma once
#include <Arduino.h>
#include <functional>

// ─── face — display-agnostic face dispatch ───────────────────────────────────
//
// Consumer registers a renderer once at boot, then calls renderFace(state)
// from loop(). The face lib dispatches to whichever implementation was
// registered — swap the implementation without changing any other code.
//
// ─── Usage ───────────────────────────────────────────────────────────────────
//   #include <face.h>
//   #include <emo_face.h>                    // or any other implementation
//
//   void setup() {
//       face_register(emo_face_renderer());  // register emo-face
//   }
//
//   void loop() {
//       renderFace(STATE_IDLE);              // dispatches to emo-face
//   }
//
// ─── States ───────────────────────────────────────────────────────────────────
// These match RobotState in TaraCore. Defined here independently so face.h
// has no dependency on TaraCore.

enum FaceState {
    FACE_BOOTING,
    FACE_CONNECTING,
    FACE_REGISTERING,
    FACE_WAITING_CONFIG,
    FACE_CONFIGURING,
    FACE_IDLE,
    FACE_LISTENING,
    FACE_THINKING,
    FACE_SPEAKING,
    FACE_SLEEPING,
    FACE_ERROR,
    FACE_GIGGLE,
    FACE_STATE_COUNT
};

// ─── Renderer interface ───────────────────────────────────────────────────────
// A FaceRenderer maps every FaceState to a function.
// Implementation libraries fill this struct and return it from a factory fn.

struct FaceRenderer {
    // render[state] is called by renderFace(state).
    // Each entry may be nullptr — renderFace() silently skips nullptr slots.
    std::function<void()> render[FACE_STATE_COUNT];
};

// ─── Registration ─────────────────────────────────────────────────────────────
// Call once in setup() with the renderer returned by the chosen implementation.
void face_register(const FaceRenderer& renderer);

// ─── Dispatch ─────────────────────────────────────────────────────────────────
// Call in loop(). Dispatches to the registered renderer for the given state.
void renderFace(FaceState state);

// ─── Convenience helpers ──────────────────────────────────────────────────────
// Cast from integer (e.g. RobotState enum from TaraCore) to FaceState.
// Use when driving renderFace() directly from TaraCore's currentState.
inline FaceState toFaceState(int robotState) {
    if (robotState < 0 || robotState >= FACE_STATE_COUNT) return FACE_IDLE;
    return static_cast<FaceState>(robotState);
}
