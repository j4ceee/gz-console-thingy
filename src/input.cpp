#include <Windows.h>

#include "addresses.h"
#include "graphics.h"
#include "input.h"

#include "log.h"
#include "game/game_world.h"
#include "imgui/ui.h"
#include "patches/ui_patches.h"

static constexpr float   FONT_SIZE_INPUT    = 0.0155f;

void Input::Draw(ID3D11DeviceContext* context)
{

    if (!context) return;

    const uint32_t game_state = *(uint32_t *)GetAddress(gz::VAR_GAME_STATE);

    if (game_state != 11) {
        // don't draw anything if we are not in-game
        return;
    }

    const auto ui = gz::UI::Get();
    gz::ConsoleSettings& settings = ui->GetSettings();

    if (!ui->IsVisible() && settings.showHint) {
        // draw input hint
        Graphics::Get()->DrawString(
            context,
            gz::ConsoleSettings::GetKeyName(settings.toggleUIKey),
            0.0078f,
            0.965f,
            FONT_SIZE_INPUT,
            0x5EC9C9C9
        );
    }
}

bool Input::FeedEvent(uint32_t message, WPARAM wParam, LPARAM lParam)
{
    const auto &ui = gz::UI::Get();
    const auto& settings = ui->GetSettings();

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
            auto* gameWorld = gz::CGameWorld::instance();
            if (gameWorld)
            {
                gameWorld->TeleportToAimPosition();
                return true;
            }
        }

        // shortcut to hide/show HUD
        if (wParam == settings.hideHUDKey && gz::UIPatches::IsInitialized())
        {
            gz::UIPatches::ToggleHideHUD();
            return true;
        }
    }

    // let other input pass through
    return false;
}