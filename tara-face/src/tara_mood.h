#pragma once

// ─── Mood constants ───────────────────────────────────────────────────────────
#define TARA_DEFAULT  0   // no eyelids — neutral open eyes
#define TARA_TIRED    1   // inner-top eyelid droops (triangles from inner corner)
#define TARA_ANGRY    2   // outer-top eyelid drops  (triangles from outer corner)
#define TARA_HAPPY    3   // bottom eyelid rises      (rounded rect from bottom)

// ─── MoodLids ─────────────────────────────────────────────────────────────────
// Holds the current and target heights for all three eyelid types.
// TaraFace tweens current → target each frame for smooth transitions.

struct MoodLids {
    float tired = 0,  tiredT  = 0;   // tired  eyelid: current / target
    float angry = 0,  angryT  = 0;   // angry  eyelid: current / target
    float happy = 0,  happyT  = 0;   // happy  eyelid: current / target

    // Set targets for the given mood. Pass eyeH/2 as maxLid.
    void setMood(uint8_t mood, float maxLid) {
        tiredT = angryT = happyT = 0;   // reset all first
        if (mood == TARA_TIRED)  tiredT = maxLid;
        if (mood == TARA_ANGRY)  angryT = maxLid;
        if (mood == TARA_HAPPY)  happyT = maxLid;
    }

    // Tween current values toward targets. Call once per frame.
    void tick() {
        tired = (tired + tiredT) / 2.0f;
        angry = (angry + angryT) / 2.0f;
        happy = (happy + happyT) / 2.0f;
    }
};
