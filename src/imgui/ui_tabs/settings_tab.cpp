#include "../ui.h"
#include "../fonts/IconsMaterialDesign.h"

#include <imgui.h>

namespace gz::UITabs
{

void RenderSettingsTab()
{
    UI* ui = UI::Get();
    ConsoleSettings& settings = ui->GetSettings();

// -- HOTKEYS --
    if (ImGui::CollapsingHeader(ICON_MD_KEYBOARD " Hotkeys", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Click a button and press a key to rebind");
        ImGui::Spacing();

        // -- UI TOGGLE HOTKEY --
        ImGui::Text("Toggle UI:");
        ImGui::SameLine();

        std::string toggleUILabel = ui->m_isCapturingToggleUI ?
            "Press a key...##toggleui" : (ConsoleSettings::GetKeyName(settings.toggleUIKey) + "##toggleui");

        if (ImGui::Button(toggleUILabel.c_str(), ImVec2(120, 0))) {
            ui->m_isCapturingToggleUI = true;
            ui->m_isCapturingTeleportKey = false;
            ui->m_isCapturingHideHUDKey = false;
        }
        UI::HoverTooltip("Click to change the hotkey for toggling the UI");

        // -- TELEPORT TO AIM HOTKEY --
        ImGui::Text("Teleport to Aim:");
        ImGui::SameLine();

        std::string teleportKeyLabel = ui->m_isCapturingTeleportKey ?
            "Press a key...##teleport" : (ConsoleSettings::GetKeyName(settings.teleportToAimKey) + "##teleport");

        if (ImGui::Button(teleportKeyLabel.c_str(), ImVec2(120, 0))) {
            ui->m_isCapturingTeleportKey = true;
            ui->m_isCapturingToggleUI = false;
            ui->m_isCapturingHideHUDKey = false;
        }
        UI::HoverTooltip("Click to change the hotkey for teleporting to aim position");

        // -- HIDE HUD HOTKEY --
        ImGui::Text("Hide HUD:");
        ImGui::SameLine();

        std::string hideHudKeyLabel = ui->m_isCapturingHideHUDKey ?
            "Press a key...##hud" : (ConsoleSettings::GetKeyName(settings.hideHUDKey) + "##hud");

        if (ImGui::Button(hideHudKeyLabel.c_str(), ImVec2(120, 0))) {
            ui->m_isCapturingTeleportKey = false;
            ui->m_isCapturingToggleUI = false;
            ui->m_isCapturingHideHUDKey = true;
        }
        UI::HoverTooltip("Click to change the hotkey for hiding the HUD");

        ImGui::Spacing();

        // -- RESET TO DEFAULTS BUTTON --
        if (ImGui::Button("Reset to Defaults")) {
            settings.toggleUIKey = VK_F1;
            settings.teleportToAimKey = VK_F2;
            settings.hideHUDKey = VK_F3;
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
        ImGui::SameLine();
        UI::HelpMarker("Reset hotkeys to F1 (Toggle UI), F2 (Teleport to Aim) and F3 (Hide HUD)");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // -- UI SETTINGS --
    if (ImGui::CollapsingHeader(ICON_MD_SETTINGS " UI Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Checkbox("Show Debug Info", &settings.showDebugInfo)) {
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
        if (ImGui::Checkbox("Show Console Thingy Hint", &settings.showHint)) {
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        } ImGui::SameLine(); UI::HelpMarker("Show or hide the hotkey hint in the bottom-left corner of the screen");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::Separator();

    ImGui::Spacing();
    ImGui::Spacing();

    // -- CREDITS --
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.98f, 0.26f, 0.26f, 1.00f)); // red color
    ImGui::Text("Generation Zero Console Thingy");
    ImGui::PopStyleColor();

    ImGui::Spacing();

    ImGui::Text("Developed by:");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("jacee", "https://github.com/j4ceee");

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::Text("Based on...");

    ImGui::Spacing();
    ImGui::Bullet();
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("Just Cause 4 Console Thingy", "https://github.com/aaronkirkham/jc4-console-thingy");
    ImGui::SameLine();
    ImGui::Text("by");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("Aaron Kirkham", "https://kirkh.am/");

    ImGui::Spacing();
    ImGui::Bullet();
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("Generation Zero Cheat Table##2", "https://fearlessrevolution.com/viewtopic.php?t=33780");
    ImGui::SameLine();
    ImGui::Text("by aSwedishMagyar and sanitka");

    ImGui::Spacing();
    ImGui::Bullet();
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("Generation Zero Cheat Table##1", "https://fearlessrevolution.com/viewtopic.php?t=8996");
    ImGui::SameLine();
    ImGui::Text("by pigeon");

    ImGui::Spacing();
    ImGui::Bullet();
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("ImGui", "https://github.com/ocornut/imgui");
}
} // namespace gz::UITabs