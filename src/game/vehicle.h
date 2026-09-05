#pragma once

#include "damageable.h"

#pragma pack(push, 1)
namespace gz
{
    class CPfxVehicle
    {

    };

    class CVehicle
    {
    public:
        void*   vtable;             // 0x00
        char    _pad1[0x2BC];       // 0x08 → 0x2C4 (CDamageable)
        char    _pad2[0x1220];      // 0x298 → 0x14E4
        int     occupancyState;     // 0x14E4

        [[nodiscard]] bool IsPlayerInVehicle() const
        {
            return occupancyState == 0;
        }

        [[nodiscard]] CDamageable* GetDamageable()
        {
            return reinterpret_cast<CDamageable*>(this);
        }
    };
    static_assert(offsetof(CVehicle, occupancyState) == 0x14E4);
}// namespace gz
#pragma pack(pop)