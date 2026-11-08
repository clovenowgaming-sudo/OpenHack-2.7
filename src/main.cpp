#include "hacks.hpp"
#include <imgui-cocos.hpp>
#include <imgui.h>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace {
    bool g_menuOpen = false;
    ImVec2 g_buttonPos(-1.f, -1.f);
    float uiScale() { return std::max(0.5f, (float) Mod::get()->getSettingValue<double>("ui-scale")); }

    void applyScale() {
        float s = uiScale();
        ImGui::GetStyle() = ImGuiStyle();
        ImGui::StyleColorsDark();
        ImGui::GetStyle().WindowRounding = 8.f;
        ImGui::GetStyle().FrameRounding = 6.f;
        ImGui::GetStyle().ScaleAllSizes(s);
#if defined(IMGUI_HAS_TEXTURES)
        ImGui::GetStyle().FontScaleMain = s;
#else
        ImGui::GetIO().FontGlobalScale = s;
#endif
    }

    // Round draggable button. Returns true when tapped (not dragged).
    bool floatingButton() {
        auto& io = ImGui::GetIO();
        float radius = 22.f * uiScale();
        if (g_buttonPos.x < 0.f) g_buttonPos = ImVec2(radius * 1.5f, io.DisplaySize.y * 0.5f);

        ImGui::SetNextWindowPos(ImVec2(g_buttonPos.x - radius, g_buttonPos.y - radius));
        ImGui::SetNextWindowSize(ImVec2(radius * 2.f, radius * 2.f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
        ImGui::Begin("##oh_button", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNav);

        ImGui::InvisibleButton("##oh_btn", ImVec2(radius * 2.f, radius * 2.f));
        bool active = ImGui::IsItemActive();
        bool hovered = ImGui::IsItemHovered();
        static float dragDist = 0.f;
        if (ImGui::IsItemActivated()) dragDist = 0.f;
        if (active) {
            ImVec2 d = io.MouseDelta;
            dragDist += std::fabs(d.x) + std::fabs(d.y);
            if (dragDist > 8.f * uiScale()) {
                g_buttonPos.x = std::clamp(g_buttonPos.x + d.x, radius, io.DisplaySize.x - radius);
                g_buttonPos.y = std::clamp(g_buttonPos.y + d.y, radius, io.DisplaySize.y - radius);
            }
        }
        bool clicked = ImGui::IsItemDeactivated() && dragDist <= 8.f * uiScale();

        auto* dl = ImGui::GetWindowDrawList();
        ImVec2 c(g_buttonPos.x, g_buttonPos.y);
        ImU32 fill = g_menuOpen ? IM_COL32(60, 170, 90, 230) : (active || hovered ? IM_COL32(70, 120, 220, 240) : IM_COL32(40, 40, 50, 220));
        dl->AddCircleFilled(c, radius, fill, 48);
        dl->AddCircle(c, radius, IM_COL32(255, 255, 255, 200), 48, 2.f);
        const char* label = "OH";
        ImVec2 ts = ImGui::CalcTextSize(label);
        dl->AddText(ImVec2(c.x - ts.x * 0.5f, c.y - ts.y * 0.5f), IM_COL32(255, 255, 255, 255), label);

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        return clicked;
    }

    struct Ui {
        bool changed = false;
        void check(const char* label, bool& v) { changed |= ImGui::Checkbox(label, &v); }
        void sliderF(const char* id, float& v, float lo, float hi, const char* fmt) {
            ImGui::SetNextItemWidth(-1.f);
            changed |= ImGui::SliderFloat(id, &v, lo, hi, fmt);
        }
        void sliderI(const char* id, int& v, int lo, int hi, const char* fmt) {
            ImGui::SetNextItemWidth(-1.f);
            changed |= ImGui::SliderInt(id, &v, lo, hi, fmt);
        }
    };

    void menuWindow() {
        auto& s = oh::state();
        auto& io = ImGui::GetIO();
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.6f, io.DisplaySize.y * 0.85f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.2f, io.DisplaySize.y * 0.07f), ImGuiCond_FirstUseEver);

        bool open = true;
        Ui ui;
        if (ImGui::Begin("OpenHack-2.7", &open)) {
            if (ImGui::BeginTabBar("##tabs")) {
                if (ImGui::BeginTabItem("Global")) {
                    ui.check("Speedhack", s.speedhack);
                    ui.sliderF("##speed", s.speed, 0.05f, 4.f, "Speed: %.2fx");
                    if (ImGui::Button("Reset speed")) { s.speed = 1.f; ui.changed = true; }
                    ImGui::Separator();
                    ui.check("Safe Mode (don't save completions while cheating)", s.safeMode);
                    ui.check("Freeze Jumps Counter", s.freezeJumps);
                    ui.check("Freeze Attempts Counter", s.freezeAttempts);
                    ui.check("Music Unlocker (practice music)", s.musicUnlock);
                    ui.check("Unlock All Icons / Colors / Items", s.unlockAll);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Level")) {
                    ui.check("Noclip", s.noclip);
                    if (s.noclip) {
                        ImGui::Indent();
                        ui.check("Player 1", s.noclipP1);
                        ui.check("Player 2", s.noclipP2);
                        ui.check("Noclip Death Limit", s.noclipLimit);
                        if (s.noclipLimit) ui.sliderI("##nclimit", s.noclipLimitCount, 1, 500, "Allow %d hits per attempt");
                        ImGui::Unindent();
                    }
                    ImGui::Separator();
                    ui.check("Show Hitboxes", s.hitboxes);
                    ui.check("Respawn Delay", s.respawnDelay);
                    if (s.respawnDelay) ui.sliderF("##rdelay", s.respawnDelayTime, 0.f, 3.f, "Delay: %.2f s");
                    ui.check("Auto Kill", s.autoKill);
                    if (s.autoKill) ui.sliderF("##akpct", s.autoKillPercent, 1.f, 100.f, "Restart at %.0f%%");
                    ui.check("No Camera Shake", s.noShake);
                    ui.check("No Camera Zoom", s.noZoom);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Cosmetic")) {
                    ui.check("Accurate Percentage (2 decimals)", s.accuratePct);
                    ui.check("RGB Icons", s.rgbIcons);
                    if (s.rgbIcons) ui.sliderF("##rgbspd", s.rgbSpeed, 0.05f, 3.f, "RGB speed: %.2f");
                    ui.check("No Death Effect", s.noDeathEffect);
                    ui.check("No Respawn Flash", s.noSpawnFlash);
                    ui.check("No Particles", s.noParticles);
                    ui.check("Solid Wave Trail", s.solidWave);
                    ui.check("No Wave Trail", s.noWaveTrail);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Bypass")) {
                    ui.check("Text Bypass (no length limit)", s.textBypass);
                    ui.check("Character Filter (allow all characters)", s.charFilter);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            if (ui.changed) s.save();
        }
        ImGui::End();
        if (!open) g_menuOpen = false;
    }
}

$on_mod(Loaded) {
    oh::state().load();

    ImGuiCocos::get().setup([] {
        applyScale();
    }).draw([] {
        if (floatingButton()) g_menuOpen = !g_menuOpen;
        if (g_menuOpen) menuWindow();
        // Only block game input while the menu is open
        ImGuiCocos::get().setInputMode(g_menuOpen ? ImGuiCocos::InputMode::Blocking : ImGuiCocos::InputMode::Default);
    });

    ImGuiCocos::get().setForceLegacy(Mod::get()->getSettingValue<bool>("legacy-render"));
    geode::listenForSettingChanges<bool>("legacy-render", [](bool v) {
        ImGuiCocos::get().setForceLegacy(v);
    });
}
