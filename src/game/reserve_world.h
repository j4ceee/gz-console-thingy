#pragma once

#include "animal_spotting_manager.h"

#pragma pack(push, 1)
namespace gz
{
    class CReserveWorld
    {
    public:
        char                    _pad0[0x58];        // 0x00 → 0x58
        CAnimalSpottingManager* m_SpottingManager;  // 0x58 → 0x60

        static CReserveWorld* instance()
        {
            return *(CReserveWorld**)GetAddress(INST_RESERVE_WORLD);
        }

        CAnimalSpottingManager* GetSpottingManager() const
        {
            return m_SpottingManager;
        }
    };
} // namespace gz
#pragma pack(pop)