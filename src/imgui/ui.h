#pragma once

#include "imgui.h"
#include "ui_settings.h"

#include <string>
#include <Windows.h>

#include "imgui_internal.h"

namespace gz
{

    class UI
    {
    public:
        static UI* Get()
        {
            static UI instance;
            return &instance;
        }

        void Initialize();
        void SetupUiLook();
        void Shutdown();
        void Render();

        bool IsVisible() const { return m_visible; }
        void SetVisible(bool visible);
        void ToggleVisible() { SetVisible(!m_visible); }

        ConsoleSettings& GetSettings() { return m_settings; }
        static bool HandleHotkeyCapture(WPARAM wParam);

        // helper functions for tabs to use
        static void HelpMarker(const char* desc, const char* warning = nullptr);
        static void HoverTooltip(const char* text, const char* warning = nullptr);
        static void InfoText(const char* text, const char* warning = nullptr);
        static void WarningText(const char* text, bool wrap = false);

        bool m_isCapturingToggleUI = false;
        bool m_isCapturingTeleportKey = false;
        bool m_isCapturingHideHUDKey = false;

    private:
        UI() = default;
        ~UI() = default;

        bool m_visible = false;
        bool m_initialized = false;
        ImFont* mainfont = nullptr;

        ConsoleSettings m_settings;

        // ImGui settings handlers
        static void* SettingsHandlerReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name);
        static void SettingsHandlerReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
        static void SettingsHandlerWriteAll(ImGuiContext*, ImGuiSettingsHandler*, ImGuiTextBuffer* buf);
    };

    // tab rendering functions (implemented in separate files)
    namespace UITabs
    {
        void RenderPlayerTab();
        void RenderWorldTab();
        void RenderSettingsTab();
    }

} // namespace gz