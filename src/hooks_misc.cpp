#include "hacks.hpp"
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/HardStreak.hpp>
#include <Geode/modify/CCParticleSystem.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/CCTextInputNode.hpp>
#include <climits>
#include <cstring>

using namespace geode::prelude;

// ---- Speedhack -------------------------------------------------------------
class $modify(OHScheduler, CCScheduler) {
    void update(float dt) {
        auto& s = oh::state();
        if (s.speedhack && s.speed > 0.f) dt *= s.speed;
        CCScheduler::update(dt);
    }
};

// ---- Camera triggers -------------------------------------------------------
class $modify(OHBaseGameLayer, GJBaseGameLayer) {
    void shakeCamera(float duration, float strength, float interval) {
        if (oh::state().noShake) return;
        GJBaseGameLayer::shakeCamera(duration, strength, interval);
    }

    void updateZoom(float zoom, float duration, int easing, float rate, int uniqueID, int controlID) {
        if (oh::state().noZoom) return;
        GJBaseGameLayer::updateZoom(zoom, duration, easing, rate, uniqueID, controlID);
    }
};

// ---- Player cosmetics ------------------------------------------------------
class $modify(OHPlayerObject, PlayerObject) {
    void playDeathEffect() {
        if (oh::state().noDeathEffect) return;
        PlayerObject::playDeathEffect();
    }

    void playSpawnEffect() {
        if (oh::state().noSpawnFlash) return;
        PlayerObject::playSpawnEffect();
    }
};

class $modify(OHHardStreak, HardStreak) {
    void updateStroke(float dt) {
        auto& s = oh::state();
        if (s.noWaveTrail) {
            this->clear();
            return;
        }
        if (s.solidWave) m_isSolid = true;
        HardStreak::updateStroke(dt);
    }
};

class $modify(OHParticles, CCParticleSystem) {
    bool addParticle() {
        if (oh::state().noParticles) return false;
        return CCParticleSystem::addParticle();
    }
};

// ---- Stats / unlocks -------------------------------------------------------
class $modify(OHStats, GameStatsManager) {
    void incrementStat(char const* key, int amount) {
        auto& s = oh::state();
        if (key) {
            if (s.freezeJumps && std::strcmp(key, "1") == 0) return;
            if (s.freezeAttempts && std::strcmp(key, "2") == 0) return;
        }
        GameStatsManager::incrementStat(key, amount);
    }

    bool isItemUnlocked(UnlockType type, int id) {
        auto& s = oh::state();
        if (s.unlockAll) return true;
        if (s.musicUnlock && type == UnlockType::GJItem && id == 17) return true;
        return GameStatsManager::isItemUnlocked(type, id);
    }
};

class $modify(OHGameManager, GameManager) {
    bool isIconUnlocked(int id, IconType type) {
        if (oh::state().unlockAll) return true;
        return GameManager::isIconUnlocked(id, type);
    }

    bool isColorUnlocked(int id, UnlockType type) {
        if (oh::state().unlockAll) return true;
        return GameManager::isColorUnlocked(id, type);
    }
};

// ---- Text bypass -----------------------------------------------------------
class $modify(OHTextInput, CCTextInputNode) {
    void setAllowedChars(gd::string filter) {
        if (oh::state().charFilter) {
            std::string all;
            for (int c = 32; c < 127; ++c) all.push_back((char) c);
            filter = all;
        }
        CCTextInputNode::setAllowedChars(filter);
    }

    bool onTextFieldInsertText(CCTextFieldTTF* sender, char const* text, int len, enumKeyCodes key) {
        if (oh::state().textBypass) m_maxLabelLength = INT_MAX / 2;
        return CCTextInputNode::onTextFieldInsertText(sender, text, len, key);
    }
};
