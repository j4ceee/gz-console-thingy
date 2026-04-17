#include "patches.h"
#include "addresses.h"

#include "graphics.h"
#include "input.h"
#include "log.h"
#include "imgui/ui.h"

#include <iostream>
#include <__msvc_ostream.hpp>
#include <imgui.h>

#include "game/animal_network.h"
#include "game/camera_collision_modifier.h"
#include "game/clock.h"
#include "game/event_scheduler.h"
#include "game/debug_logger.h"
#include "game/deep_water.h"
#include "game/environment_gfx.h"
#include "game/game_state.h"
#include "game/intro.h"
#include "game/player_eq_utils.h"
#include "game/ui_manager.h"
#include "game/ui_static_handler.h"
#include "game/weapon_consumption.h"
#include "game/camera/camera_director.h"
#include "patches/building_patches.h"
#include "patches/resource_patch.h"
#include "patches/vehicle_patches.h"
#include "patches/cloud_patch.h"
#include "patches/dlc_patches.h"
#include "patches/fasttravel_patches.h"
#include "patches/map_zoom_patches.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace gz
{
LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);

static decltype(WndProc) *                pfn_WndProc     = nullptr;

LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    const auto &ui = UI::Get();

    bool isInGame = GameState::IsInGame();

    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    if (ui->IsVisible()) {
        ImGuiIO& io = ImGui::GetIO();

        // block mouse messages
        if (io.WantCaptureMouse) {
            switch (msg) {
                case WM_MOUSEWHEEL: // allow this for changing camera distance in third person RC
                    break;
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

    if (isInGame) {
        if (Input::FeedEvent(msg, wParam, lParam)) {
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

    // basic sanity check
    if (strcmp((const char *)GetAddress(SANITY_CHECK), "Avalanche Engine") != 0) {
        return false;
    }
    Log("Sanity check passed, setting up hooks...");

    if (MH_Initialize() != MH_OK)
    {
        Log("Failed to initialize MinHook");
        return false;
    }

    Log("Hooking WndProc...");
    // WndProc
    bool success = MH_CreateHookGZ(
        WND_PROC,
        &WndProc,
        &pfn_WndProc
    );

    if (!success)
    {
        Log("Failed to hook WndProc");
        return false;
    }

    Log("WndProc hooked successfully");

    // Graphics::Flip
    success = MH_CreateHookGZ(
        GRAPHICS_FLIP,
        &Graphics::GraphicsFlipCallback,
        &Graphics::pfn_Flip
    );

    if (!success)
    {
        Log("Failed to hook Graphics::Flip");
        return false;
    }

    // game patches
    try {
        if (!CCameraCollisionModifier::SetupCheckCollisionHook())
            Log("Failed to setup Camera Distance Patch");

        if (!CClock::SetupUpdateGameHook())
            Log("Failed to setup Clock UpdateGame hook");

        if (!SetupLoggingHooks())
            Log("Failed to setup logging hooks");

        if (!PlayerEqUtils::SetupGetBackpackWeightHook())
            Log("Failed to setup GetBackpackWeight hook");

        if (!PlayerEqUtils::SetupGetEquipmentWeightHook())
            Log("Failed to setup GetEquipmentWeight hook");

        if (!CCameraDirector::SetupPushCameraHook())
            Log("Failed to setup CameraDirector PushCamera hook");

        if (!CDeepWaterHandling::SetupGetWaterHeightHook())
            Log("Failed to setup Deep Water Cached Height hook");

        if (!Utils::SetupHashFunction()) {
            // required for several patches -> fail initialization
            throw std::runtime_error("Failed to setup hash function");
        }

        if (!CSpawnedAnimalNetworkComponent::SetupHackRequestHook())
            Log("Failed to setup Animal Hack Request hook");

        if (!CRemoteController::SetupSignalStrengthHook())
            Log("Failed to setup Remote Controller Signal Strength hook");

        if (!EventManager::Initialize())
            Log("Failed to setup EventManager hook");

        if (!SetupWeatherUpdateHook())
            Log("Failed to setup WeatherUpdate hook");

        if (!AmmoDeployableConsumption::SetupAmmoHook())
            Log("Failed to setup Ammo Consumption hook");

        if (!AmmoDeployableConsumption::SetupDeployableHook())
            Log("Failed to setup Deployable Consumption hook");

        EventTimePatch::Initialize();
        BuildingPatches::Initialize();
        ResourcePatches::Initialize();
        VehiclePatches::Initialize();
        CloudPatch::Initialize();
        FastTravelPatches::Initialize();
        if (DLCPatch::Initialize())
            DLCPatch::EnableDLCBoundaryBypass(); // always keep enabled (enables players to enter DLC areas)
        if (MapZoomPatch::Initialize())
            MapZoomPatch::DisableMapZoomLimit(); // always keep enabled (removes max zoom cap)

        if (!Intro::SetupIntroComplete())
            Log("Failed to setup 'Intro Complete'");

        if (!SUIScene::SetupUpdateVisibilityShowHook())
            Log("Failed to setup UpdateVisibilityShow hook");

        if (!CUIManager::SetupIsUIShownHook())
            Log("Failed to setup IsUIShown hook");

    } catch (const std::exception &e) {
        Log("Exception during patch initialization: %s", e.what());
        return false;
    }

    return true;
}
} // namespace gz
