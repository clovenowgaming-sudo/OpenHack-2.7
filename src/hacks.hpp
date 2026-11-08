#pragma once
#include <Geode/Geode.hpp>
#include <vector>

namespace oh {
    struct State {
        // Player
        bool noclip = false, noclipP1 = true, noclipP2 = true;
        bool noclipLimit = false; int noclipLimitCount = 50;
        bool safeMode = true;
        // Global
        bool speedhack = false; float speed = 1.f;
        bool freezeJumps = false, freezeAttempts = false;
        bool musicUnlock = false, unlockAll = false;
        // Level
        bool hitboxes = false;
        bool respawnDelay = false; float respawnDelayTime = 0.5f;
        bool autoKill = false; float autoKillPercent = 50.f;
        bool noShake = false, noZoom = false;
        // Cosmetic
        bool accuratePct = false;
        bool rgbIcons = false; float rgbSpeed = 0.5f;
        bool noDeathEffect = false, noSpawnFlash = false, noParticles = false;
        bool solidWave = false, noWaveTrail = false;
        // Bypass
        bool textBypass = false, charFilter = false;

        bool anyCheat() const { return noclip || (speedhack && speed != 1.f) || unlockAll || musicUnlock; }
        void load();
        void save() const;
    };

    State& state();

    struct Hsv { static cocos2d::ccColor3B toRgb(float h, float s, float v); };
}
