#pragma once

#include "addresses.h"
#include "hook_helpers.h"

#pragma pack(push, 1)
namespace gz::PlayerEqUtils
{
    inline bool g_ignoreEncumbrance = false;
    inline bool g_unlimitedStorageSize = false;

    // -- backpack weight hook -----------------------------------------------------------------------------------------
    using GetBackpackWeightFunc = float(*)();
    inline GetBackpackWeightFunc g_getBackpackWeight = nullptr;

    static bool IsIgnoringEncumbrance()
    {
        return g_ignoreEncumbrance;
    }

    static void SetIgnoreEncumbrance(bool ignore)
    {
        g_ignoreEncumbrance = ignore;
    }

    static float HookedGetBackpackWeight()
    {
        if (g_ignoreEncumbrance || g_unlimitedStorageSize || g_getBackpackWeight == nullptr)
        { // if either unlimited carry weight or unlimited storage size is enabled -> ignore backpack weight
            return 0.0f;
        }
        return g_getBackpackWeight();
    }
    inline bool SetupGetBackpackWeightHook()
    {
        if (g_getBackpackWeight != nullptr)
        {
            return true;
        }
        return MH_CreateHookGZ(GET_BACKPACK_WEIGHT, &HookedGetBackpackWeight, &g_getBackpackWeight);
    }

    // -- all containers weight hook -----------------------------------------------------------------------------------
    using GetEquipmentWeightFunc = float(*)(void* container);
    inline GetEquipmentWeightFunc g_getEquipmentWeight = nullptr;

    static bool IsUnlimitedStorageSize()
    {
        return g_unlimitedStorageSize;
    }

    static void SetUnlimitedStorageSize(bool unlimited)
    {
        g_unlimitedStorageSize = unlimited;
    }

    static float HookedGetEquipmentWeight(void* container)
    {
        if (g_unlimitedStorageSize || g_getEquipmentWeight == nullptr)
        {
            return 0.0f;
        }
        return g_getEquipmentWeight(container);
    }
    inline bool SetupGetEquipmentWeightHook()
    {
        if (g_getEquipmentWeight != nullptr)
        {
            return true;
        }
        return MH_CreateHookGZ(GET_EQUIPMENT_WEIGHT, &HookedGetEquipmentWeight, &g_getEquipmentWeight);
    }
} // namespace gz::PlayerEqUtils
#pragma pack(pop)
