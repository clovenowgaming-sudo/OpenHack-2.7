#include "hacks.hpp"
#include <Geode/modify/PlayLayer.hpp>
#include <algorithm>

using namespace geode::prelude;

class $modify(OHPlayLayer, PlayLayer) {
    struct Fields {
        int blocked = 0;
        bool respawnPending = false;
        bool ourReset = false;
        bool killed = false;
        float hue = 0.f;
    };

    void destroyPlayer(PlayerObject* player, GameObject* object) override {
        auto& s = oh::state();
        auto* f = m_fields.self();
        if (s.noclip && object != m_anticheatSpike) {
            bool p1 = player == m_player1;
            if ((p1 && s.noclipP1) || (!p1 && s.noclipP2)) {
                if (!s.noclipLimit) return;
                if (++f->blocked <= s.noclipLimitCount) return;
                // limit exceeded: let the death through
            }
        }
        PlayLayer::destroyPlayer(player, object);

        if (s.respawnDelay && player && player->m_isDead && !f->respawnPending) {
            f->respawnPending = true;
            this->runAction(CCSequence::create(
                CCDelayTime::create(std::max(0.f, s.respawnDelayTime)),
                CCCallFunc::create(this, callfunc_selector(OHPlayLayer::ohRespawn)),
                nullptr));
        }
    }

    void ohRespawn() {
        m_fields->ourReset = true;
        this->delayedResetLevel();
    }

    void ohKill() { this->resetLevel(); }

    // Game's own delayed reset: swallowed while Respawn Delay is on (we schedule our own).
    void delayedResetLevel() {
        auto* f = m_fields.self();
        if (oh::state().respawnDelay && !f->ourReset) return;
        f->ourReset = false;
        f->respawnPending = false;
        PlayLayer::delayedResetLevel();
    }

    void resetLevel() {
        auto* f = m_fields.self();
        f->blocked = 0;
        f->killed = false;
        f->respawnPending = false;
        f->ourReset = false;
        PlayLayer::resetLevel();
    }

    void updateDebugDrawSettings() {
        PlayLayer::updateDebugDrawSettings();
        if (oh::state().hitboxes) {
            m_isDebugDrawEnabled = true;
            if (m_debugDrawNode) m_debugDrawNode->setVisible(true);
        }
    }

    void postUpdate(float dt) override {
        PlayLayer::postUpdate(dt);
        auto& s = oh::state();
        auto* f = m_fields.self();

        if (s.hitboxes && !m_isDebugDrawEnabled) {
            m_isDebugDrawEnabled = true;
            if (m_debugDrawNode) m_debugDrawNode->setVisible(true);
        }

        if (s.rgbIcons && m_player1) {
            f->hue += dt * s.rgbSpeed;
            auto c1 = oh::Hsv::toRgb(f->hue, 1.f, 1.f);
            auto c2 = oh::Hsv::toRgb(f->hue + 0.5f, 1.f, 1.f);
            m_player1->setColor(c1);
            m_player1->setSecondColor(c2);
            if (m_player2) {
                m_player2->setColor(c2);
                m_player2->setSecondColor(c1);
            }
        }

        if (s.autoKill && !f->killed && m_player1 && !m_player1->m_isDead &&
            this->getCurrentPercent() >= s.autoKillPercent) {
            f->killed = true;
            this->runAction(CCCallFunc::create(this, callfunc_selector(OHPlayLayer::ohKill)));
        }
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        if (oh::state().accuratePct && m_percentageLabel && m_level && !m_level->isPlatformer()) {
            m_percentageLabel->setString(fmt::format("{:.2f}%", this->getCurrentPercent()).c_str());
        }
    }

    void levelComplete() {
        auto& s = oh::state();
        if (s.safeMode && s.anyCheat()) {
            // Marks the run as a test run so no progress/stats are saved.
            m_isTestMode = true;
        }
        PlayLayer::levelComplete();
    }
};
