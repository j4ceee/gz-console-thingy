#pragma once

#include <atomic>

namespace gz::TpState
{
    inline std::atomic<bool> g_toggleRequested{false};

    // --- resource priming (player_spawn_manager.h) ---
    inline bool g_primingResources = false;
    inline bool g_resourcesPrimed = false;

    // --- animation layers (character.h / TpLayers) ---
    inline void* g_layerOwner = nullptr; // character parked instances belong to. they die with it, so a character swap (world reload, new save) has to reset the install state
    inline bool g_layersInstalled = false;
    inline bool g_animationsActive = false;

    // --- camera (camera/third_person_camera.h) ---
    inline bool g_cameraActive = false;
}
