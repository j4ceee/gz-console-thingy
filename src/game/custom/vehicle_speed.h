#pragma once

#include "addresses.h"
#include "hook_helpers.h"

namespace gz::VehicleSpeed
{
    inline float g_VehicleSpeedMultiplier = 1.0f;

    using GetVehicleTorqueOnWheel_t = float(__fastcall*)(void* pThis, int wheelIdx);
    inline GetVehicleTorqueOnWheel_t g_getVehicleTorqueOnWheel = nullptr;

    inline float __fastcall HookedGetVehicleTorqueOnWheel(void* pThis, int wheelIdx)
    {
        return g_getVehicleTorqueOnWheel(pThis, wheelIdx) * g_VehicleSpeedMultiplier;
    }

    inline bool SetupVehicleTorqueOnWheelHook()
    {
        if (g_getVehicleTorqueOnWheel != nullptr) return true;
        return MH_CreateHookGZ(GET_VEHICLE_TORQUE, &HookedGetVehicleTorqueOnWheel, &g_getVehicleTorqueOnWheel);
    }


    inline bool g_NoAirResistance = false;

    using CalculateLandAerodynamics_t = void(__fastcall*)(void*, void*, void*, float*);
    inline CalculateLandAerodynamics_t g_calculateLandAerodynamics = nullptr;

    inline void __fastcall HookedCalculateLandAerodynamics(void* pThis, void* pBody, void* pMotion, float* pResult)
    {
        g_calculateLandAerodynamics(pThis, pBody, pMotion, pResult);

        if (g_NoAirResistance && pResult != nullptr)
        {
            pResult[0] = 0.0f;
            pResult[1] = 0.0f;
            pResult[2] = 0.0f;
            pResult[3] = 0.0f;
            pResult[4] = 0.0f;
            pResult[5] = 0.0f;
            pResult[6] = 0.0f;
            pResult[7] = 0.0f;
        }
    }

    inline bool SetupCalculateLandAerodynamicsHook()
    {
        if (g_calculateLandAerodynamics != nullptr) return true;
        return MH_CreateHookGZ(CALC_LAND_AERODYNAMICS, &HookedCalculateLandAerodynamics, &g_calculateLandAerodynamics);
    }
}
