#pragma once

#include "animal.h"
#include "animal_population.h"
#include "log.h"
#include "map.h"

#pragma pack(push, 1)
namespace gz
{
    class CAnimalSpawnSystem
    {
    public:
        // currently unused
        void SpawnAnimal(CAnimal* animal, void* networkComponent)
        {
            meow_hook::func_call<void>(GetAddress(SPAWN_ANIMAL), this, animal, networkComponent);
        }

        // splits "hunt_classc_load02" into base "hunt_classc" + loadout=1
        static int SplitLoadoutTag(const char* fullTag, std::string& outBaseTag)
        {
            std::string tag(fullTag);
            size_t pos = tag.rfind("_load");
            if (pos == std::string::npos) { outBaseTag = tag; return 0; }
            int loadout = std::atoi(tag.c_str() + pos + 5) - 1; // "load01" -> 0, "load02" -> 1, ...
            if (loadout < 0) { outBaseTag = tag; return 0; }
            outBaseTag = tag.substr(0, pos);
            return loadout;
        }

        CAnimal* SpawnAtPosition(CAnimalPopulationManager* popManager, const char* spawnTag, const CVector3f& position, const bool forceSpawn = true)
        {
            std::string baseTag;
            int loadout = SplitLoadoutTag(spawnTag, baseTag);

            CAnimalPopulation* population = popManager->FindPopulation(baseTag.c_str());
            if (!population)
            {
                Log("[ANIMAL SPAWN] no population for base tag '%s'", baseTag.c_str());
                return nullptr;
            }

            CVector2f mapPos = CMap::WorldToMapPosition(position.x, position.z);

            CAnimalGroup* group = population->AddNewAnimalGroup(popManager, mapPos, (std::byte)loadout, this);
            if (!group)
            {
                Log("[ANIMAL SPAWN] AddNewAnimalGroup failed for base tag '%s'", baseTag.c_str());
                return nullptr;
            }

            // pull the animal out of the freshly created group
            auto& groupAnimals = *reinterpret_cast<std::vector<CAnimal*>*>(&group->m_Animals);
            if (groupAnimals.empty())
            {
                Log("[ANIMAL SPAWN] group has no animals");
                return nullptr;
            }
            CAnimal* animal = groupAnimals[0];

            if (forceSpawn)
            {
                // mark position (bits 0x10 + 0x20) so SpawnAnimal trusts it directly instead of running navmesh search
                animal->m_MapPosition = mapPos;
                animal->m_ScriptedSpawnPositionY = position.y;
                animal->m_Flags = (std::byte)((uint8_t)animal->m_Flags | 0x10 | 0x20);
            }
            return animal;
        }
    };
} // namespace gz
#pragma pack(pop)
