#include "ui.h"
#include "log.h"
#include "fonts/roboto-regular.cpp"
#include "fonts/materialicons.cpp"

#include "game/player_manager.h"
#include "game/event_scheduler.h"

#include <imgui.h>
#include "imgui_internal.h"

namespace gz
{

void UI::Initialize()
{
    if (m_initialized) {
        return;
    }

    Log("Initializing ImGui...");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.IniFilename = "GZConsoleThingy.ini";

    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.MouseDrawCursor = false;

    SetupUiLook();

    // register custom settings handler
    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = "GZConsoleThingy";
    ini_handler.TypeHash = ImHashStr("GZConsoleThingy");
    ini_handler.ReadOpenFn = SettingsHandlerReadOpen;
    ini_handler.ReadLineFn = SettingsHandlerReadLine;
    ini_handler.WriteAllFn = SettingsHandlerWriteAll;
    ini_handler.UserData = this;
    ImGui::AddSettingsHandler(&ini_handler);

    m_initialized = true;
    Log("ImGui initialized successfully");
}

void UI::Shutdown()
{
    if (!m_initialized) {
        return;
    }

    Log("Shutting down ImGui");
    ImGui::DestroyContext();
    m_initialized = false;
}

void UI::SetupUiLook()
{
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    // colours
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.89f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.12f, 0.12f, 0.12f, 0.45f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.98f, 0.26f, 0.26f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.98f, 0.26f, 0.26f, 0.67f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.396f, 0.012f, 0.012f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.88f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.98f, 0.26f, 0.26f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.98f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(1.00f, 0.33f, 0.33f, 0.17f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.50f, 0.43f, 0.43f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.75f, 0.10f, 0.10f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.75f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.98f, 0.26f, 0.26f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.98f, 0.26f, 0.26f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.98f, 0.26f, 0.26f, 0.95f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.58f, 0.18f, 0.18f, 0.86f);
    colors[ImGuiCol_TabSelected]            = ImVec4(0.68f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TabSelectedOverline]    = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
    colors[ImGuiCol_TabDimmed]              = ImVec4(0.15f, 0.07f, 0.07f, 0.97f);
    colors[ImGuiCol_TabDimmedSelected]      = ImVec4(0.42f, 0.14f, 0.14f, 1.00f);

    // rounding
    style.FrameRounding = 4.0f;
    style.WindowRounding = 6.0f;

    // fonts
    ImGuiIO& io = ImGui::GetIO();
    mainfont = io.Fonts->AddFontFromMemoryCompressedTTF(
        Roboto_Regular_compressed_data,
        Roboto_Regular_compressed_size,
        16.0f
    );

    ImFontConfig config;
    config.MergeMode = true;
    io.Fonts->AddFontFromMemoryCompressedTTF(
        MaterialIcons_compressed_data,
        MaterialIcons_compressed_size,
        12.0f,
        &config
    );

    // set main font scale
    style.ScaleAllSizes(1.2f);
    style.FontScaleMain = 1.2f;
}

void UI::SetVisible(bool visible)
{
    m_visible = visible;

    if (visible) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = true;
    } else {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = false;
    }
}

bool UI::HandleHotkeyCapture(WPARAM wParam)
{
    UI* ui = Get();

    if (!ui->m_isCapturingToggleUI && !ui->m_isCapturingTeleportKey) {
        return false;
    }

    if (!ConsoleSettings::IsValidKey(static_cast<int>(wParam))) {
        return true;
    }

    if (ui->m_isCapturingToggleUI) {
        ui->m_settings.toggleUIKey = static_cast<int>(wParam);
        ui->m_isCapturingToggleUI = false;
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        Log("Toggle UI key bound to: %s", ConsoleSettings::GetKeyName(static_cast<int>(wParam)).c_str());
    }
    else if (ui->m_isCapturingTeleportKey) {
        ui->m_settings.teleportToAimKey = static_cast<int>(wParam);
        ui->m_isCapturingTeleportKey = false;
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        Log("Teleport to Aim key bound to: %s", ConsoleSettings::GetKeyName(static_cast<int>(wParam)).c_str());
    }
    else if (ui->m_isCapturingHideHUDKey) {
        ui->m_settings.hideHUDKey = static_cast<int>(wParam);
        ui->m_isCapturingHideHUDKey = false;
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        Log("Hide HUD key bound to: %s", ConsoleSettings::GetKeyName(static_cast<int>(wParam)).c_str());
    }

    return true;
}

void UI::Render()
{
    if (!m_visible || !m_initialized) {
        return;
    }

    const uint32_t game_state = *(uint32_t *)GetAddress(VAR_GAME_STATE);
    if (game_state != 11) {
        SetVisible(false);
        return;
    }

    ImGui::Begin("Generation Zero Console Thingy", &m_visible);

    CNetworkPlayerManager& playerMgr = CNetworkPlayerManager::instance();

    ImGui::PushFont(mainfont);
    ImGui::Spacing();
    ImGui::Text("Hello, %s!", playerMgr.GetLocalPlayerProfileName().c_str());
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Player")) {
            ImGui::BeginChild("PlayerContent", ImVec2(0, 0), false);
            ImGui::Spacing();
            UITabs::RenderPlayerTab();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("World")) {
            ImGui::BeginChild("WorldContent", ImVec2(0, 0), false);
            ImGui::Spacing();
            UITabs::RenderWorldTab();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Settings")) {
            ImGui::BeginChild("SettingsContent", ImVec2(0, 0), false);
            ImGui::Spacing();
            UITabs::RenderSettingsTab();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::PopFont();
    ImGui::End();
}

// Helper functions
void UI::HelpMarker(const char* desc, const char* warning)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        InfoText(desc, warning);
        ImGui::EndTooltip();
    }
}

void UI::HoverTooltip(const char* text, const char* warning)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        InfoText(text, warning);
        ImGui::EndTooltip();
    }
}

void UI::InfoText(const char* text, const char* warning)
{
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(text);

    if (warning)
    {
        ImGui::Separator();
        WarningText(warning);
    }

    ImGui::PopTextWrapPos();
}

void UI::WarningText(const char* text, bool wrap)
{
    if (wrap) ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s", text);
    if (wrap) ImGui::PopTextWrapPos();
}

// ImGui settings handlers
void* UI::SettingsHandlerReadOpen(ImGuiContext*, ImGuiSettingsHandler* handler, const char* name)
{
    if (strcmp(name, "Settings") == 0) {
        UI* ui = (UI*)handler->UserData;
        return &ui->m_settings;
    }
    return nullptr;
}

void UI::SettingsHandlerReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
{
    auto* settings = (ConsoleSettings*)entry;
    int i;

    if (sscanf_s(line, "ToggleUIKey=%d", &i) == 1) {
        settings->toggleUIKey = i;
    }
    else if (sscanf_s(line, "TeleportToAimKey=%d", &i) == 1) {
        settings->teleportToAimKey = i;
    }
    else if (sscanf_s(line, "HideHUDKey=%d", &i) == 1) {
        settings->hideHUDKey = i;
    }
    else if (sscanf_s(line, "ShowDebugInfo=%d", &i) == 1) {
        settings->showDebugInfo = (i != 0);
    }
    else if (sscanf_s(line, "ShowCTHint=%d", &i) == 1) {
        settings->showHint = (i != 0);
    }
    else if (sscanf_s(line, "DisableAutoEvents=%d", &i) == 1) {
        settings->disableAutoEvents = (i != 0);
        SetSchedulerBlocked(settings->disableAutoEvents);
    }
}

void UI::SettingsHandlerWriteAll(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{
    UI* ui = (UI*)handler->UserData;
    const ConsoleSettings& settings = ui->m_settings;

    buf->appendf("[%s][Settings]\n", handler->TypeName);
    buf->appendf("ToggleUIKey=%d\n", settings.toggleUIKey);
    buf->appendf("TeleportToAimKey=%d\n", settings.teleportToAimKey);
    buf->appendf("HideHUDKey=%d\n", settings.hideHUDKey);
    buf->appendf("ShowDebugInfo=%d\n", settings.showDebugInfo ? 1 : 0);
    buf->appendf("ShowCTHint=%d\n", settings.showHint ? 1 : 0);
    buf->appendf("DisableAutoEvents=%d\n", settings.disableAutoEvents ? 1 : 0);
    buf->append("\n");
}
} // namespace gz