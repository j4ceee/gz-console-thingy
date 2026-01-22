#pragma once

#pragma pack(push, 1)
namespace gz
{
    class CVehicle
    {
    public:
        void*   vtable;             // 0x00
        char    _pad08[0x2BC];      // 0x08 → 0x2C4 (CDamageable)
        char    _pad298[0x1220];    // 0x298 → 0x14E4
        int     occupancyState;     // 0x14E4

        bool IsPlayerInVehicle()
        {
            return occupancyState == 0;
        }

        CDamageable* GetDamageable() { return reinterpret_cast<CDamageable*>(this); }
    };
}// namespace gz
#pragma pack(pop)