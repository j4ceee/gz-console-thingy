#pragma once
#include <algorithm>

#include "meow_hook/detour.h"
#include "addresses.h"

#pragma pack(push, 1)
namespace gz::AmmoDeployableConsumption
{
    using ConsumeAmmoFunc = void(*)(void* weaponComponent, int32_t amount);
    using ConsumeDeployablesFunc = void(*)(void* param1, uint32_t param2, uint8_t param3, uint8_t param4);

    inline ConsumeAmmoFunc g_consumeAmmo = nullptr;
    inline ConsumeDeployablesFunc g_consumeDeployables = nullptr;

    inline bool ammoConsumptionDisabled = false;
    inline bool deployableConsumptionDisabled = false;

    inline void HookedConsumeAmmo(void* weaponComponent, int32_t amount)
    {
        if (!ammoConsumptionDisabled && g_consumeAmmo) {
            g_consumeAmmo(weaponComponent, amount);
        }
    }

    inline void HookedConsumeDeployables(void* param1, uint32_t param2, uint8_t param3, uint8_t param4)
    {
        if (!deployableConsumptionDisabled && g_consumeDeployables) {
            g_consumeDeployables(param1, param2, param3, param4);
        }
    }

    inline void SetupAmmoHook()
    {
        g_consumeAmmo = MH_STATIC_DETOUR(GetAddress(CONSUME_AMMO), HookedConsumeAmmo);
    }

    inline void SetupDeployableHook()
    {
        g_consumeDeployables = MH_STATIC_DETOUR(GetAddress(CONSUME_DEPLOYABLES), HookedConsumeDeployables);
    }

    inline bool IsAmmoConsumptionDisabled()
    {
        return ammoConsumptionDisabled;
    }

    inline void ToggleAmmoConsumption()
    {
        ammoConsumptionDisabled = !ammoConsumptionDisabled;
    }

    inline bool IsDeployableConsumptionDisabled()
    {
        return deployableConsumptionDisabled;
    }

    inline void ToggleDeployableConsumption()
    {
        deployableConsumptionDisabled = !deployableConsumptionDisabled;
    }

    inline bool IsAmmoHookInitialized()
    {
        return g_consumeAmmo != nullptr;
    }

    inline bool IsDeployableHookInitialized()
    {
        return g_consumeDeployables != nullptr;
    }
}
#pragma pack(pop)