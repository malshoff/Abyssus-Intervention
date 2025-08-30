#include "AimbotTab.h"
#include <imgui.h>
#include "../../dev/imgui/IconsFontAwesome5.h"

#include "../../cheats/Core/Config.h"
#include "../../cheats/Utils/Input.h"
#include <dev/logger.h>

namespace CheatMenu { namespace Tabs {

void AimbotTab() {
    ImGui::Text("%s  Aimbot", ICON_FA_CROSSHAIRS);
    ImGui::Separator();

    // Toggle row with iconized label
    ImGui::PushID("aimbot_enabled");
    bool enabled = Cheat::Config::Aimbot::enabled;
    const char* label = enabled ? ICON_FA_TOGGLE_ON "  Enabled" : ICON_FA_TOGGLE_OFF "  Disabled";
    if (ImGui::Selectable(label, false, 0, ImVec2(0, 26))) {
        Cheat::Config::Aimbot::enabled = !enabled;
        LOG_INFO("GUI: Aimbot checkbox clicked - new value: %s", Cheat::Config::Aimbot::enabled ? "ENABLED" : "DISABLED");
        LOG_INFO("GUI: Variable address: %p", &Cheat::Config::Aimbot::enabled);
    }
    ImGui::PopID();

    if (ImGui::Checkbox("Visibility Check", &Cheat::Config::Aimbot::visibilityCheck)) {
        LOG_INFO("GUI: Aimbot Visibility Check %s", Cheat::Config::Aimbot::visibilityCheck ? "ENABLED" : "DISABLED");
    }

    ImGui::Spacing();
    ImGui::Text("Aim Target");
    int aimTarget = static_cast<int>(Cheat::Config::Aimbot::aimTarget);
    const char* aimItems[] = { "bones", "center" };
    if (ImGui::Combo("##aim_target_combo", &aimTarget, aimItems, IM_ARRAYSIZE(aimItems))) {
        Cheat::Config::Aimbot::aimTarget = static_cast<Cheat::Config::Aimbot::AimTarget>(aimTarget);
        LOG_INFO("GUI: Aimbot aim target set to %s", aimTarget == 0 ? "bones" : "center");
    }

    ImGui::Spacing();
    ImGui::Text("Vertical Aim Offset");
    ImGui::SameLine();
    if (ImGui::SliderFloat("##aim_vert_offset", &Cheat::Config::Aimbot::aimVerticalOffset, -100.0f, 100.0f, "%.1f")) {
        LOG_INFO("GUI: Aimbot vertical offset = %.1f", Cheat::Config::Aimbot::aimVerticalOffset);
    }


    ImGui::Spacing();
    ImGui::Text("Smoothing");
    if (ImGui::Checkbox("Enable Smoothing", &Cheat::Config::Aimbot::smoothEnabled)) {
        LOG_INFO("GUI: Aimbot smoothing %s", Cheat::Config::Aimbot::smoothEnabled ? "ENABLED" : "DISABLED");
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    if (ImGui::SliderFloat("Amount##smoothing_amount", &Cheat::Config::Aimbot::maxTurnSpeed, 100.0f, 5000.0f, "%.0f")) {
        LOG_INFO("GUI: Aimbot smoothing amount (max turn speed) = %.0f", Cheat::Config::Aimbot::maxTurnSpeed);
    }



    ImGui::Spacing();
    ImGui::Text("Hotkey Configuration");
    ImGui::Separator();

    ImGui::Text("Aimbot Trigger Key:");
    ImGui::SameLine();

    if (Cheat::Config::Hotkeys::IsCapturingHotkey &&
        Cheat::Config::Hotkeys::CurrentHotkeyBeingSet == &Cheat::Config::Hotkeys::AimbotTrigger) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Press any key... (ESC to cancel)");
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
            BYTE capturedKey = Cheat::Utils::Input::CaptureNextKeyPress();
            if (capturedKey != 0) {
                if (capturedKey == VK_ESCAPE) {
                    Cheat::Config::Hotkeys::IsCapturingHotkey = false;
                    Cheat::Config::Hotkeys::CurrentHotkeyBeingSet = nullptr;
                    LOG_INFO("GUI: Aimbot hotkey capture cancelled");
                } else {
                    Cheat::Config::Hotkeys::AimbotTrigger = capturedKey;
                    Cheat::Config::Hotkeys::IsCapturingHotkey = false;
                    Cheat::Config::Hotkeys::CurrentHotkeyBeingSet = nullptr;
                    LOG_INFO("GUI: Aimbot trigger key set to: %s (VK: %d)",
                        Cheat::Utils::Input::GetKeyName(capturedKey), capturedKey);
                }
            }
        }
    } else {
        ImGui::Text("Current aimbot key: %s", Cheat::Utils::Input::GetKeyName(Cheat::Config::Hotkeys::AimbotTrigger));
        ImGui::SameLine();
        if (ImGui::Button("Set Hotkey")) {
            Cheat::Config::Hotkeys::IsCapturingHotkey = true;
            Cheat::Config::Hotkeys::CurrentHotkeyBeingSet = &Cheat::Config::Hotkeys::AimbotTrigger;
            LOG_INFO("GUI: Started capturing aimbot trigger key");
        }
    }

    ImGui::Spacing();
    ImGui::Text("Targeting Settings");
    ImGui::SliderFloat("Max Distance", &Cheat::Config::Aimbot::maxDistance, 1000.0f, 100000.0f);
}

} } // namespace CheatMenu::Tabs

