#include <Windows.h>

#include "graphics.h"
#include "input.h"

#include "log.h"
#include "game/camera_collision_modifier.h"
#include "game/game_state.h"
#include "game/game_world.h"
#include "game/input_manager.h"
#include "game/ui_manager.h"
#include "imgui/ui.h"

static constexpr float FONT_SIZE_INPUT = 0.0155f;

void Input::Draw(ID3D11DeviceContext* context)
{
    if (!context) return;

    if (!gz::GameState::IsInGame()) {
        // don't draw anything if we are not in-game
        return;
    }

    const auto ui = gz::UI::Get();
    gz::ConsoleSettings& settings = ui->GetSettings();

    if (!ui->IsVisible() && settings.showHint) {
        // draw input hint
        const std::string keyName = gz::ConsoleSettings::GetKeyName(settings.toggleUIKey);
        const char* hint = gz::NInput::g_isUsingGamepad ? "LS + RS" : keyName.c_str();

        Graphics::Get()->DrawString(
            context, hint, 0.0078f, 0.965f, FONT_SIZE_INPUT, 0x5EC9C9C9
        );
    }
}

bool Input::FeedEvent(uint32_t message, WPARAM wParam, LPARAM lParam)
{
    // track active input device
    if (message == WM_KEYDOWN || message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN)
        gz::NInput::g_isUsingGamepad = false;

    gz::UI* ui = gz::UI::Get();
    gz::ConsoleSettings& settings = ui->GetSettings();

    // if UI is visible, block ALL game input except UI toggle
    if (ui->IsVisible()) {
        if (message == WM_KEYDOWN && (lParam >> 30) == 0)
        {
            // handle hotkey capture (check if user is rebinding a key)
            if (gz::UI::HandleHotkeyCapture(wParam)) {
                return true;
            }

            // toggle UI off
            if (wParam == settings.toggleUIKey) {
                ui->ToggleVisible();
                return true;
            }
        }
        // block everything else from reaching the game
        return true;
    }

    // ui is not visible, handle global hotkeys
    if (message == WM_KEYDOWN && (lParam >> 30) == 0)
    {
        // toggling UI
        if (wParam == settings.toggleUIKey) {
            ui->ToggleVisible();
            return true;
        }

        // shortcut to teleport to aim position
        if (wParam == settings.teleportToAimKey) {
            if (auto* gameWorld = gz::CGameWorld::instance())
            {
                gameWorld->TeleportToAimPosition();
                return true;
            }
        }

        // shortcut to hide/show HUD
        if (wParam == settings.hideHUDKey)
        {
            gz::CUIManager::SetHideHud(!gz::CUIManager::IsHudHidden());
            // save setting
            settings.hideHud = gz::CUIManager::IsHudHidden();
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            return true;
        }

        if (wParam == settings.thirdPersonKey)
        {
            gz::TpState::g_toggleRequested.store(true, std::memory_order_relaxed);
            return true;
        }
    }

    if (message == WM_MOUSEWHEEL)
    {
        auto* mgr = gz::CNetworkPlayerManager::instance();
        if (mgr && mgr->GetPlayer())
        {
            auto* character = mgr->GetPlayer()->GetCharacter();
            if (character && character->IsControllingEntity())
            {
                float delta = GET_WHEEL_DELTA_WPARAM(wParam);
                float normalizedDelta = delta / WHEEL_DELTA;
                gz::CCameraCollisionModifier::ChangeThirdPersonCameraDistance(-normalizedDelta * 0.05f);
                return true; // consume the event
            }
        }
    }

    // let other input pass through
    return false;
}