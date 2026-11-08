#include "hacks.hpp"
#include <cmath>
#include <algorithm>

using namespace geode::prelude;

namespace oh {
    State& state() { static State s; return s; }

    ccColor3B Hsv::toRgb(float h, float s, float v) {
        h = h - std::floor(h);
        float r = 0, g = 0, b = 0;
        int i = (int) (h * 6.f);
        float f = h * 6.f - i, p = v * (1.f - s), q = v * (1.f - f * s), t = v * (1.f - (1.f - f) * s);
        switch (i % 6) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            default: r = v; g = p; b = q; break;
        }
        return { (GLubyte) (r * 255), (GLubyte) (g * 255), (GLubyte) (b * 255) };
    }

#define OH_FIELDS(X) \
    X(bool, noclip) X(bool, noclipP1) X(bool, noclipP2) X(bool, noclipLimit) X(int, noclipLimitCount) \
    X(bool, safeMode) X(bool, speedhack) X(float, speed) X(bool, freezeJumps) X(bool, freezeAttempts) \
    X(bool, musicUnlock) X(bool, unlockAll) X(bool, hitboxes) X(bool, respawnDelay) X(float, respawnDelayTime) \
    X(bool, autoKill) X(float, autoKillPercent) X(bool, noShake) X(bool, noZoom) X(bool, accuratePct) \
    X(bool, rgbIcons) X(float, rgbSpeed) X(bool, noDeathEffect) X(bool, noSpawnFlash) X(bool, noParticles) \
    X(bool, solidWave) X(bool, noWaveTrail) X(bool, textBypass) X(bool, charFilter)

    void State::load() {
        auto* m = Mod::get();
#define X(T, N) N = m->getSavedValue<T>(#N, N);
        OH_FIELDS(X)
#undef X
    }

    void State::save() const {
        auto* m = Mod::get();
#define X(T, N) m->setSavedValue<T>(#N, N);
        OH_FIELDS(X)
#undef X
    }
}
