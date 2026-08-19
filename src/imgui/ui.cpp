#include "ui.h"
#include "log.h"
#include "fonts/roboto-regular.cpp"
#include "fonts/materialicons.cpp"

#include "game/player_network_manager.h"

#include <imgui.h>
#include "imgui_internal.h"
#include "game/damageable.h"
#include "game/game_state.h"
#include "game/input_manager.h"

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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.MouseDrawCursor = false;

    SetupUiLook();

    // set default settings
    m_settings.ApplyDefaults();

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
    ImGuiIO& io = ImGui::GetIO();

    // query DPI from the game window (or fallback to desktop DPI)
    float dpiScale = 1.0f;
    dpiScale = GetDeviceCaps(GetDC(nullptr), LOGPIXELSX) / 96.0f;
    float fontSize = std::round(16.0f * dpiScale); // e.g. 32px on 4K

    // fonts
    mainfont = io.Fonts->AddFontFromMemoryCompressedTTF(
        Roboto_Regular_compressed_data,
        Roboto_Regular_compressed_size,
        fontSize
    );

    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;

    io.Fonts->AddFontFromMemoryCompressedTTF(
        MaterialIcons_compressed_data,
        MaterialIcons_compressed_size,
        0.75f * fontSize,
        &config
    );

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

    style.WindowPadding     = ImVec2(14.0f, 12.0f);
    style.FramePadding      = ImVec2( 8.0f,  5.0f);
    style.ItemSpacing       = ImVec2(10.0f,  7.0f);
    style.ItemInnerSpacing  = ImVec2( 7.0f,  5.0f);
    style.CellPadding       = ImVec2( 6.0f,  4.0f);
    style.IndentSpacing     = 20.0f;
    style.ScrollbarSize     = 12.0f;
    style.GrabMinSize       = 12.0f;

    style.ScaleAllSizes(0.75f * dpiScale); // scales padding, grab sizes, etc. proportionally
    style.MouseCursorScale = dpiScale;
}

void UI::SetVisible(bool visible)
{
    m_visible = visible;

    auto& io = ImGui::GetIO();
    if (visible)
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange; // let ImGui change cursor shapes
    else
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;  // hands cursor back to game

    if (auto* inputMgr = NInput::CManager::instance())
    {
        if (visible) inputMgr->LoseFocus();
        else inputMgr->GainFocus();
    }
}

bool UI::HandleHotkeyCapture(WPARAM wParam)
{
    UI* ui = Get();

    if (!ui->m_isCapturingToggleUI && !ui->m_isCapturingTeleportKey && !ui->m_isCapturingHideHUDKey) {
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
    if (!m_initialized) {
        return;
    }

    // --- CONTROLLER TOGGLE LOGIC ---
    // check for L3 + R3 (Left Stick Click + Right Stick Click)
    static bool s_wasComboPressed = false;
    const bool isComboPressed = ImGui::IsKeyDown(ImGuiKey_GamepadL3) && ImGui::IsKeyDown(ImGuiKey_GamepadR3);

    // only trigger once per press to prevent flickering open / closed
    if (isComboPressed && !s_wasComboPressed) {
        SetVisible(!m_visible);
    }
    s_wasComboPressed = isComboPressed;
    // -------------------------------

    if (!m_visible) {
        return;
    }

    if (!GameState::IsInGame()) {
        SetVisible(false);
        return;
    }

    ImGui::Begin("Generation Zero Console Thingy");
    ImGui::PushFont(mainfont);

    auto* playerMgr = CNetworkPlayerManager::instance();

    if (!playerMgr) {
        ImGui::Text("Error: CNetworkPlayerManager instance not found.");
        ImGui::PopFont();
        ImGui::End();
        return;
    }

    auto* networkPlayer = playerMgr->GetNetworkPlayer();

    ImGui::Spacing();
    ImGui::Text("Hello, %s!", networkPlayer->GetProfileName().c_str());
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Player")) {
            ImGui::BeginChild("PlayerContent", ImVec2(0, 0), false);
            ImGui::Spacing();
            UITabs::RenderPlayerTab(playerMgr);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Machines")) {
            ImGui::BeginChild("MachineContent", ImVec2(0, 0), false);
            ImGui::Spacing();
            UITabs::RenderMachineTab(playerMgr);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("World")) {
            ImGui::BeginChild("WorldContent", ImVec2(0, 0), false);
            ImGui::Spacing();
            UITabs::RenderWorldTab(playerMgr);
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
    RenderHelpMarkerButton();

    if (ImGui::BeginItemTooltip())
    {
        InfoText(desc, warning);
        ImGui::EndTooltip();
    }
}
// help marker with array of bullet points for warning
void UI::HelpMarker(const char* desc, const std::vector<const char*>& warnings)
{
    RenderHelpMarkerButton();

    if (ImGui::BeginItemTooltip())
    {
        InfoText(desc, warnings);
        ImGui::EndTooltip();
    }
}

void UI::RenderHelpMarkerButton()
{
    static int s_counter = 0;
    static int s_lastFrame = -1;

    if (const int frame = ImGui::GetFrameCount(); frame != s_lastFrame) {
        s_counter = 0;
        s_lastFrame = frame;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::PushID(s_counter++);
    ImGui::SmallButton("(?)");
    ImGui::PopID();
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
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
// info text with array of bullet points for warning
void UI::InfoText(const char* text, const std::vector<const char*>& warnings)
{
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(text);

    if (!warnings.empty())
    {
        ImGui::Separator();
        for (const char* warning : warnings)
        {
            StartWarningText();
            ImGui::Bullet();
            ImGui::SameLine();
            ImGui::Text("%s", warning);
            EndWarningText();
        }
    }
}

void UI::StartWarningText()
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
}

void UI::EndWarningText()
{
    ImGui::PopStyleColor();
}

void UI::WarningText(const char* text, bool wrap)
{
    StartWarningText();
    if (wrap)
    {
        ImGui::TextWrapped("%s", text);
    }
    else
    {
        ImGui::Text("%s", text);
    }
    EndWarningText();
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
    settings->ReadLine(line);
}

void UI::SettingsHandlerWriteAll(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{
    UI* ui = (UI*)handler->UserData;
    buf->appendf("[%s][Settings]\n", handler->TypeName);
    ui->m_settings.WriteAll(buf);
    buf->append("\n");
}

void UI::GetHealthModification(CDamageable* damageable, const char* identifier, const char* invulTip, const char* healthNote,
const char* reviveLabel, const char* reviveTip, std::function<void()> onRevive)
{
    std::string identifierStr = std::string(identifier);
    int currentHealth = damageable->GetHealth();
    int maxHealth = damageable->GetMaxHealth();
    int healthPercent = damageable->GetHealthInPercentage();

    ImGui::Text("Current: %d / %d (%d%%)", currentHealth, maxHealth, healthPercent);

    if (!damageable->IsDestroyed() || identifierStr != "character") // only characters can be revived
    {
        if (ImGui::SliderInt(("Health (%)##" + identifierStr).c_str(), &healthPercent, 0, 100)) {
            damageable->SetHealthInPercentage(healthPercent);
        }
        ImGui::SameLine();
        if (strlen(healthNote) > 0) {
            UI::HelpMarker("Sets health as a percentage of max health.", healthNote);
        } else
        {
            UI::HelpMarker("Sets health as a percentage of max health.");
        }
        if (ImGui::Button(("Restore Full Health##" + identifierStr).c_str())) {
            damageable->RestoreHealth();
        }
    }
    // if destroyed (and not a vehicle), show revive button
    else
    {
        if (ImGui::Button((reviveLabel + std::string("##") + identifierStr).c_str())) {
            onRevive();
        }
        ImGui::SameLine();
        UI::HelpMarker(reviveTip);
    }

    // invulnerability
    bool invulnerable = damageable->IsInvulnerable();
    if (ImGui::Checkbox(("Invulnerable##" + identifierStr).c_str(), &invulnerable)) {
        damageable->SetInvulnerable(invulnerable, identifierStr == "character" ? 'p' : 0);
        // save setting for vehicles
        if (identifierStr == "vehicle")
        {
            UI* ui = UI::Get();
            ConsoleSettings& settings = ui->GetSettings();
            settings.enableInfiniteBikeHealth = invulnerable;
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
    }
    ImGui::SameLine();
    UI::HelpMarker(invulTip);
}
} // namespace gz