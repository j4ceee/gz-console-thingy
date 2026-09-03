#pragma once

#include <cstdint>
#include "game/environment_gfx.h"

namespace gz::PresetOverride
{
    // Priorities from settings/weathersettings.bin:
    // - 1-8        ambient weather (windy, overcast, snow, rain, forced_rain, north...)
    // - 100        underground_interior
    // - 500        colour-grading effects
    // - 1000-06    vision modes (night vision, thermal, xray, scouting, huntermate)
    // - 2000       end_game
    //
    // Filter override: above everything

    // IDs are huge on purpose: the game's own IDs are preset array indices -> ours never collide + CWeather::Update will never remove them
    inline constexpr uint32_t kFilterId = 0x57A10002;
    inline constexpr uint32_t kFilterPrio = 2100;

    struct Slot
    {
        uint32_t id;
        uint32_t priority;
        uint32_t activeHash = 0;
        int32_t activeIndex = -1;
        float transitionTime = -1.0f;
    };


    inline Slot g_filter{kFilterId, kFilterPrio};

    /// transitionTime < 0 uses the game's default (g_PresetsBlendingTime)
    inline bool Apply(Slot& slot, uint32_t presetHash, float transitionTime = -1.0f)
    {
        auto* mgr = CEnvironmentGfxManager::instance();
        if (!mgr) return false;

        const int32_t index = mgr->FindPresetIndexByHash(presetHash);
        if (index < 0) return false;

        void** handle = mgr->GetPresetHandle(index);
        if (!handle) return false;

        if (slot.activeHash) mgr->RemoveParameterSet(slot.id); // fade the old one out

        mgr->AddParameterSet(handle, 1.0f, slot.id, slot.priority, transitionTime);
        slot.activeHash = presetHash;
        slot.activeIndex = index;
        slot.transitionTime = transitionTime;
        return true;
    }

    inline void Clear(Slot& slot)
    {
        if (auto* mgr = CEnvironmentGfxManager::instance())
            mgr->RemoveParameterSet(slot.id);
        slot.activeHash = 0;
        slot.activeIndex = -1;
    }

    /// m_ParamSets is wiped when the gfx system is released (menu / level reload)
    /// -> re-add if our set vanished while we still think it's active
    inline void Reapply(Slot& slot)
    {
        if (!slot.activeHash) return;
        auto* mgr = CEnvironmentGfxManager::instance();
        if (!mgr || mgr->HasParamSet(slot.id)) return;

        if (void** handle = mgr->GetPresetHandle(slot.activeIndex))
            mgr->AddParameterSet(handle, 1.0f, slot.id, slot.priority, slot.transitionTime);
    }

    inline bool ApplyFilter(uint32_t hash, float t = -1.0f) { return Apply(g_filter, hash, t); }
    inline void ClearFilter() { Clear(g_filter); }

    inline bool IsFilterActive() { return g_filter.activeHash != 0; }
    inline uint32_t ActiveFilterHash() { return g_filter.activeHash; }

    inline void OnGameUpdate()
    {
        Reapply(g_filter);
    }
}