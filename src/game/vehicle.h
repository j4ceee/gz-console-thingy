#pragma once

#include "addresses.h"
#include "damageable.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
    class CPfxVehicle
    {
    public:
        char    _pad0[0x2FC];           // 0x000 → 0x2FC
        float   m_OfficialTopSpeedKPH;  // 0x2FC → 0x300
    };
    static_assert(offsetof(CPfxVehicle, m_OfficialTopSpeedKPH) == 0x2FC);


    class CVehicle
    {
    public:
        void*           vtable;             // 0x0000
        char            _pad0[0x2BC];       // 0x0008 → 0x02C4 (CDamageable)
        char            _pad1[0x1220];      // 0x02C4 → 0x14E4
        int             m_LocalPlayerSeat;  // 0x14E4 → 0x14E8
        char            _pad2[0x36A0];      // 0x14E8 → 0x4B88
        CPfxVehicle*    m_PfxVehicle;       // 0x4B88 → 0x4B90
        char            _pad3[0x660];       // 0x4B90 → 0x51F0
        float           m_FuelLevel;        // 0x51F0 → 0x51F4

        [[nodiscard]] bool IsPlayerInVehicle() const
        {
            return m_LocalPlayerSeat == 0;
        }

        [[nodiscard]] CDamageable* GetDamageable()
        {
            return reinterpret_cast<CDamageable*>(this);
        }

        [[nodiscard]] bool IsCar() const
        {
            return reinterpret_cast<uintptr_t>(vtable) == GetAddress(VEHICLE_CAR_VTABLE);
        }

        [[nodiscard]] bool HasPhysicsVehicle() const
        {
            return m_PfxVehicle != nullptr;
        }

        [[nodiscard]] float GetFuelLevel() const
        {
            return m_FuelLevel;
        }

        [[nodiscard]] float GetOfficialTopSpeedMs() const
        {
            return m_PfxVehicle ? m_PfxVehicle->m_OfficialTopSpeedKPH * 0.2777778f : 0.0f;
        }

        [[nodiscard]] float GetSpeedMs() const
        {
            return meow_hook::func_call<float>(GetAddress(VEHICLE_GET_SPEED_MS), this);
        }

        void SetFuelLevel(float percent)
        {
            meow_hook::func_call<void>(GetAddress(VEHICLE_SET_FUEL_LEVEL), this, percent);
        }
    };
    static_assert(offsetof(CVehicle, m_LocalPlayerSeat) == 0x14E4);
    static_assert(offsetof(CVehicle, m_PfxVehicle) == 0x4B88);
    static_assert(offsetof(CVehicle, m_FuelLevel) == 0x51F0);
}// namespace gz
#pragma pack(pop)
