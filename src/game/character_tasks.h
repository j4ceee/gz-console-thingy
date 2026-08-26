#pragma once

#include "data_types.h"
#include "player_network_manager.h"

namespace gz::CharacterTasks
{
    inline bool  g_jumpSeededThisFrame = false;

    using JumpTaskFunc = void(__cdecl*)(void* ctx, void* taskData, void* mem);
    inline JumpTaskFunc g_origJumpOnEnter = nullptr;
    inline JumpTaskFunc g_origJumpUpdate  = nullptr;
    inline JumpTaskFunc g_origSteeringUpdate  = nullptr;

    inline float CurrentWantedY()
    {
        auto* mgr = CNetworkPlayerManager::instance();
        auto* ch  = mgr ? mgr->GetCharacter() : nullptr;
        auto* pfx = ch ? ch->GetPfxInstance() : nullptr;
        return pfx ? pfx->m_WantedVelocityWS.y : -999.0f;
    }

    inline void __cdecl JumpUpdateDetour(void* ctx, void* taskData, void* mem)
    {
        const float wyStart = CurrentWantedY();
        g_origJumpUpdate(ctx, taskData, mem);
        const float wyEnd = CurrentWantedY();

        if (wyEnd != wyStart) g_jumpSeededThisFrame = true;
    }

    inline bool SetupJumpUpdate()
    {
        if (g_origJumpUpdate != nullptr) return true;
        return MH_CreateHookGZ(FUNC_JUMP_UPDATE, &JumpUpdateDetour, &g_origJumpUpdate);
    }

    inline void __cdecl SteeringUpdateDetour(void* ctx, void* taskData, void* mem)
    {
        using namespace TpLayers;

        auto* mgr = CNetworkPlayerManager::instance();
        auto* ch  = mgr ? mgr->GetCharacter() : nullptr;
        auto* pfx = ch ? ch->GetPfxInstance() : nullptr;

        if (!g_installed || !g_tpActive || !pfx)
        {
            g_origSteeringUpdate(ctx, taskData, mem);
            return;
        }

        const float yBefore  = pfx->m_WantedVelocityWS.y;
        const bool  airborne = pfx->GetGroundDistance() > 1.0f;

        g_origSteeringUpdate(ctx, taskData, mem);

        // mirror FP behaviour: while airborne the jump owns the vertical axis
        if (g_jumpSeededThisFrame || airborne)
            pfx->m_WantedVelocityWS.y = yBefore;
    }

    inline bool SetupSteeringUpdate()
    {
        if (g_origSteeringUpdate != nullptr) return true;
        return MH_CreateHookGZ(FUNC_STEERING_UPDATE, &SteeringUpdateDetour, &g_origSteeringUpdate);
    }
}