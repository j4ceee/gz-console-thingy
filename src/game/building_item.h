#pragma once

#include <cstddef>
#include "addresses.h"
#include "data_types.h"
#include "hook_helpers.h"

#pragma pack(push, 1)
namespace gz
{

    inline bool g_forceSpawnLocationValid = false;

    class CControlPointSpawned;
    using AlignToGridFunc_t = bool(*)(CControlPointSpawned* zone, CVector3f* inPos, CMatrix4f* outMatrix, uint8_t buildType, void* dimensions, uint8_t quadrant);
    inline AlignToGridFunc_t g_origAlignToGrid = nullptr;

    class CControlPointSpawned
    {
        public:
        static bool HookedAlignToGrid(CControlPointSpawned* zone, CVector3f* inPos, CMatrix4f* outMatrix, uint8_t buildType, void* dimensions, uint8_t quadrant)
        {
            bool result = g_origAlignToGrid(zone, inPos, outMatrix, buildType, dimensions, quadrant);
            return g_forceSpawnLocationValid ? true : result;
        }

        static bool SetupAlignToGridHook()
        {
            if (g_origAlignToGrid) return true;
            return MH_CreateHookGZ(BUILDING_ALIGN_TO_GRID, &HookedAlignToGrid, &g_origAlignToGrid);
        }
    };

    class CBuildingItem;
    using GetSpawnLocationFunc_t = void(*)(CBuildingItem*, CMatrix4f*, CVector3f*);
    inline GetSpawnLocationFunc_t g_origGetSpawnLocation = nullptr;
    using CanPlaceBuildingFunc_t = bool(*)(CBuildingItem*);
    inline CanPlaceBuildingFunc_t g_origCanPlaceBuilding = nullptr;

    class CBuildingItem
    {
    public:
        char                    m_pad0[0x360];
        CControlPointSpawned*   m_BuildingZone;
        char                    m_pad1[0x018];
        bool                    m_SpawnLocationValid;

        static void HookedGetSpawnLocation(CBuildingItem* thisPtr, CMatrix4f* outMatrix, CVector3f* position)
        {
            g_origGetSpawnLocation(thisPtr, outMatrix, position);
            if (g_forceSpawnLocationValid) {
                // ignore the real grid-alignment / collision result - ghost always renders as valid
                thisPtr->m_SpawnLocationValid = true;
            }
        }

        static bool SetupGetSpawnLocationHook()
        {
            if (g_origGetSpawnLocation) return true;
            return MH_CreateHookGZ(BUILDING_ITEM_GET_SPAWN_LOCATION, &HookedGetSpawnLocation, &g_origGetSpawnLocation);
        }

        // gates actual placement
        static bool HookedCanPlaceBuilding(CBuildingItem* thisPtr)
        {
            if (g_forceSpawnLocationValid) return true; // bypass material / limit / network checks entirely
            return g_origCanPlaceBuilding(thisPtr);
        }

        static bool SetupCanPlaceBuildingHook()
        {
            if (g_origCanPlaceBuilding) return true;
            return MH_CreateHookGZ(BUILDING_CAN_PLACE_BUILDING, &HookedCanPlaceBuilding, &g_origCanPlaceBuilding);
        }
    };
    static_assert(offsetof(CBuildingItem, m_BuildingZone) == 0x360);
    static_assert(offsetof(CBuildingItem, m_SpawnLocationValid) == 0x380);
} // namespace gz
#pragma pack(pop)