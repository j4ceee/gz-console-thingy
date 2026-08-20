#pragma once

#include <map>
#include "animal_type.h"
#include "data_types.h"

#pragma pack(push, 1)
namespace gz
{
    class CAnimalSpawnSystem;
    class CAnimalPopulationManager;
    class CAnimal;
    class CAnimalHomeRange;

    struct SGroupAnimalData
    {
        std::byte m_Loadout;        // +0x00
        char      _pad[7];          // +0x01 → +0x08
        uint64_t  m_SpecialSpawnTag;// +0x08
    };
    static_assert(sizeof(SGroupAnimalData) == 0x10);

    class CAnimalPopulation
    {
    public:
        char                                _pad0[0x08];    // 0x00 → 0x08
        uint32_t                            m_NameHashId;   // 0x08 → 0x0C
        char                                _pad1[0x34];    // 0x0C → 0x40
        CAnimalType*                        m_AnimalType;   // 0x40 → 0x48
        BasicVector                         m_Animals;      // 0x48 → 0x60
        char                                _pad2[0x58];    // 0x60 → 0xB8
        BasicVector                         m_Groups;       // 0xB8 → 0xD0
        char                                _pad3[0x58];    // 0xD0 → 0x128
        std::map<int, CAnimalHomeRange*>    m_HomeRanges;   // 0x128 → 0x138

        CAnimalGroup* AddNewAnimalGroup(CAnimalPopulationManager* manager, CVector2f mapPosition, std::byte loadout, CAnimalSpawnSystem* spawnSystem);
    };
    static_assert(offsetof(CAnimalPopulation, m_Groups) == 0xB8);
    static_assert(offsetof(CAnimalPopulation, m_HomeRanges) == 0x128);

    class CAnimalPopulationManager
    {
    public:
        char        _pad0[0x50];        // 0x00 → 0x50
        BasicVector m_Populations;      // 0x50 → 0x68
        char        _pad1[0x40];        // 0x68 → 0xA8
        uint32_t    m_AnimalIdCounter;  // 0xA8 → 0xAC
        uint32_t    m_GroupIdCounter;   // 0xAC → 0xB0

        [[nodiscard]] CAnimalPopulation* FindPopulation(const char* spawnTag) const;
    };
    static_assert(offsetof(CAnimalPopulationManager, m_AnimalIdCounter) == 0xA8);
    static_assert(offsetof(CAnimalPopulationManager, m_GroupIdCounter) == 0xAC);


    inline CAnimalGroup* CAnimalPopulation::AddNewAnimalGroup(CAnimalPopulationManager* manager, CVector2f mapPosition, std::byte loadout, CAnimalSpawnSystem* spawnSystem)
    {
        std::vector<SGroupAnimalData> animals = { { loadout, {}, 0 } };

        return meow_hook::func_call<CAnimalGroup*>(
            GetAddress(POPULATION_ADD_GROUP),
            this,
            &mapPosition,
            &animals,
            (uint8_t)0,             // param_6 = false
            (uint8_t)1,             // param_7 = true
            &manager->m_AnimalIdCounter,
            &manager->m_GroupIdCounter,
            spawnSystem,
            (uint32_t)0xFFFFFFFF,   // param_8
            (uint64_t)0             // param_9, spawn-point-specific id
        );
    }


    inline CAnimalPopulation* CAnimalPopulationManager::FindPopulation(const char* spawnTag) const
    {
        auto& vec = *reinterpret_cast<const std::vector<CAnimalPopulation*>*>(&m_Populations);
        for (auto* population : vec)
        {
            if (population && population->m_AnimalType && strcmp(population->m_AnimalType->GetSpawnTag(), spawnTag) == 0)
            {
                return population;
            }
        }
        return nullptr;
    }
} // namespace gz
#pragma pack(pop)