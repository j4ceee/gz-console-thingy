#pragma once

#include <cstdint>

#include "data.h"

namespace gz::Data
{
    struct SpawnableData : public DataEntry {
        const char* tag;
        const uint32_t hash;
    };

    using SpawnableCategory = DataCategory<SpawnableData>;
} // namespace gz::Data

#include "spawnables/machines.h"
#include "spawnables/npcs.h"
#include "spawnables/vehicles.h"
#include "spawnables/deployables.h"
#include "spawnables/buildings.h"

namespace gz::Data
{
    struct SpawnCategory
    {
        const char*               displayName;
        const SpawnableCategory*  category;  // Changed to const pointer
        size_t                    count;
        char                      spawnSystemType;
    };

    static const SpawnCategory spawnCategories[] = { // type ids: 'v' = vehicles, 'm' = machines, 'd' = default
        {"Vehicles", Spawnables::all_vehicles, 3, 'v'},
        {"Items & Deployables", Spawnables::all_deployables, 2, 'd'},
        {"Buildings & Props", Spawnables::buildings, 2, 'd'},
        {"Machines", Spawnables::all_machines, 10, 'm'},
        {"NPCs", Spawnables::all_npcs, 6, 'd'},
    };
} // namespace gz::Data
