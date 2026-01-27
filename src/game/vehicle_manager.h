#pragma once

#include "addresses.h"

#pragma pack(push, 1)
namespace gz
{
    class CVehicle;

    class CVehicleManager
    {
        struct RefCountedVehicle {
            void* vtable;                          // +0x00
            int m_refCount;                        // +0x08 (used in XADD operations)
            int m_weakRefCount;                    // +0x0C (used in final cleanup)
        };

    public:
        char _pad0[0x48];                           // 0x00 → 0x48
        CVehicle*           m_pCurrentVehicle;      // +0x48
        RefCountedVehicle*  m_pVehicleRefCount;     // +0x50

        static CVehicleManager* instance()
        {
            return *(CVehicleManager**)(GetAddress(INST_VEHICLE_MANAGER));
        }

        CVehicle* GetPlayerVehicle() {
            return this->m_pCurrentVehicle;
        }
    };
} // namespace gz
#pragma pack(pop)