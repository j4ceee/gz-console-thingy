#pragma once

#include "animal_population.h"
#include "animal_spawn_system.h"
#include "animal_spotting_manager.h"

#pragma pack(push, 1)
namespace gz
{
    class CReserveWorld
    {
    public:
        char                        _pad0[0x48];        // 0x00 → 0x48
        CAnimalPopulationManager*   m_PopulationManager;// 0x48 → 0x50
        CAnimalSpawnSystem*         m_SpawnSystem;      // 0x50 → 0x58
        CAnimalSpottingManager*     m_SpottingManager;  // 0x58 → 0x60

        static CReserveWorld* instance()
        {
            return *(CReserveWorld**)GetAddress(INST_RESERVE_WORLD);
        }

        [[nodiscard]] CAnimalSpottingManager* GetSpottingManager() const
        {
            return m_SpottingManager;
        }

        [[nodiscard]] CAnimalSpawnSystem* GetSpawnSystem() const
        {
            return m_SpawnSystem;
        }

        [[nodiscard]] CAnimalPopulationManager* GetPopulationManager() const
        {
            return m_PopulationManager;
        }
    };
    static_assert(offsetof(CReserveWorld, m_SpottingManager) == 0x58);
} // namespace gz
#pragma pack(pop)