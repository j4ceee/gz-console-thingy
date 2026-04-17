#pragma once

#include "hook_helpers.h"

#pragma pack(push, 1)
namespace gz
{
    class CDeepWaterHandling;

    using GetCachedWaterHeightFunc = float(*)(CDeepWaterHandling*);
    inline GetCachedWaterHeightFunc g_getWaterHeight = nullptr;

    inline bool g_noDeepWaterEffects = false;

    class CDeepWaterHandling
    {
    public:
        static float HookedGetWaterHeight(CDeepWaterHandling* waterHandling)
        {
            if (g_noDeepWaterEffects)
            {
                return -1000.0f;
            }

            if (g_getWaterHeight)
            {
                return g_getWaterHeight(waterHandling);
            }
            return 0.0f; // fallback
        }

        static bool SetupGetWaterHeightHook()
        {
            if (g_getWaterHeight != nullptr)
            {
                return true;
            }
            return MH_CreateHookGZ(DEEP_WATER_CACHED_HEIGHT, &HookedGetWaterHeight, &g_getWaterHeight);
        }
    };
} // namespace gz
#pragma pack(pop)
