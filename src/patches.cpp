#include "patches.h"
#include "addresses.h"

#include "graphics.h"
#include "input.h"
#include "log.h"
#include "imgui/ui.h"

#include <iostream>
#include <__msvc_ostream.hpp>
#include <meow_hook/detour.h>
#include <imgui.h>

#include "game/event_scheduler.h"
#include "game/debug_logger.h"
#include "game/environment_gfx.h"
#include "game/weapon_consumption.h"
#include "patches/building_patches.h"
#include "patches/health_patch.h"
#include "patches/resource_patch.h"
#include "patches/vehicle_patches.h"
#include "patches/cloud_patch.h"
#include "patches/detection_patch.h"
#include "patches/ui_patches.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace gz
{
LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);

static decltype(WndProc) *                pfn_WndProc     = nullptr;

LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    const auto &input = Input::Get();
    const uint32_t game_state   = *(uint32_t *)GetAddress(VAR_GAME_STATE);
    const auto &ui = UI::Get();

    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    if (ui->IsVisible()) {
        ImGuiIO& io = ImGui::GetIO();

        // block mouse messages
        if (io.WantCaptureMouse) {
            switch (msg) {
                case WM_INPUT:
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_LBUTTONDBLCLK:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_RBUTTONDBLCLK:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_MBUTTONDBLCLK:
                case WM_MOUSEWHEEL:
                case WM_MOUSEHWHEEL:
                case WM_MOUSEMOVE:
                    return true;
            }
        }

        // block keyboard messages
        if (io.WantCaptureKeyboard) {
            switch (msg) {
                case WM_KEYDOWN:
                    break;
                case WM_KEYUP:
                case WM_CHAR:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                    return true;
            }
        }
    }

    if (game_state == 11) {
        if (input->FeedEvent(msg, wParam, lParam)) {
            return true;
        }
    }

    // pass to original WndProc
    return pfn_WndProc(hwnd, msg, wParam, lParam);
}

bool InitPatchesAndHooks()
{
    Log("Starting InitPatchesAndHooks");

    Log("Checking sanity check address: %p", (void *)GetAddress(SANITY_CHECK));

    // basic sanity check, ensure the game version is what we are expecting.
    // this will prevent crashes if the game updates, but someone is using an old version of this mod.
    if (strcmp((const char *)GetAddress(SANITY_CHECK), "Avalanche Engine") != 0) {
        return false;
    }
    Log("Sanity check passed, setting up hooks...");

    Log("Hooking WndProc...");
    // WndProc
    pfn_WndProc = MH_STATIC_DETOUR(GetAddress(WND_PROC), WndProc);
    Log("WndProc hooked successfully");

    // Graphics::Flip
    Graphics::pfn_Flip = MH_STATIC_DETOUR(GetAddress(GRAPHICS_FLIP), Graphics::GraphicsFlipCallback);

    // game patches
    try {
        SetupLoggingHooks();
        Utils::SetupHashFunction();
        Events::InitializeHashes();
        SetupEventManagerHook();
        SetupWeatherUpdateHook();
        AmmoDeployableConsumption::SetupAmmoHook();
        AmmoDeployableConsumption::SetupDeployableHook();

        EventTimePatch::Initialize();
        UIPatches::Initialize();
        BuildingPatches::Initialize();
        HealthPatches::Initialize();
        ResourcePatches::Initialize();
        VehiclePatches::Initialize();
        CloudPatch::Initialize();
        DetectionPatch::Initialize();
    } catch (const std::exception &e) {
        Log("Exception during patch initialization: %s", e.what());
        return false;
    }

    return true;
}
}; // namespace gz
