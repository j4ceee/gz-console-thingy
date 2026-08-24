#pragma once

#include <cstdint>
#include "addresses.h"
#include "hook_helpers.h"
#include "building_item.h"

#pragma pack(push, 1)
namespace gz
{
    class CBuildingGrid;

    using AddBuildingToGridFunc_t = bool(*)(CBuildingGrid*, void*, uint8_t, int64_t);
    inline AddBuildingToGridFunc_t g_origAddBuildingToGrid = nullptr;

    class CBuildingGrid
    {
    public:
        char        _pad0[0x148];
        int32_t*    m_CellArrayBegin;
        int32_t*    m_CellArrayEnd;
        char        _pad1[0x8];
        int32_t     m_OriginRow; // 0x160 - grid window origin row (-1 when the grid isn't centered)
        int32_t     m_OriginCol; // 0x164
        uint32_t    m_Width; // 0x168 - number of columns (row stride)

        bool ComputeFootprintBounds(int32_t outBounds[4], void* building, uint8_t quadrant, int64_t* centerRowCol)
        {
            return meow_hook::func_call<bool>(
                GetAddress(BUILDING_GRID_COMPUTE_FOOTPRINT_BOUNDS),
                this, outBounds, building, quadrant, centerRowCol
            );
        }

        [[nodiscard]] int32_t GetHeight() const
        {
            return (int32_t)((m_CellArrayEnd - m_CellArrayBegin) / m_Width);
        }

        [[nodiscard]] int32_t WorldToArrayIndex(int32_t worldCoord, bool isRow) const
        {
            if (m_OriginRow == -1 || m_OriginCol == -1) return worldCoord; // grid not centered - no translation
            if (isRow) return (GetHeight() >> 1) - m_OriginRow + worldCoord;
            return (int32_t)(m_Width >> 1) - m_OriginCol + worldCoord;
        }

        void PreClearFootprint(int32_t rowSpan, int32_t colSpan, int32_t centerRow, int32_t centerCol) const
        {
            int32_t startRow = centerRow - (rowSpan >> 1);
            int32_t startCol = centerCol - (colSpan >> 1);
            for (int32_t r = 0; r < rowSpan; r++)
            {
                int32_t arrayRow = WorldToArrayIndex(startRow + r, true);
                for (int32_t c = 0; c < colSpan; c++)
                {
                    int32_t arrayCol = WorldToArrayIndex(startCol + c, false);
                    m_CellArrayBegin[m_Width * arrayRow + arrayCol] = -1; // 0xFFFFFFFF sentinel = "empty"
                }
            }
        }

        static bool HookedAddBuildingToGrid(CBuildingGrid* thisPtr, void* buildingPtr, uint8_t quadrant, int64_t centerRowCol)
        {
            if (!g_forceSpawnLocationValid)
            {
                return g_origAddBuildingToGrid(thisPtr, buildingPtr, quadrant, centerRowCol);
            }

            int32_t bounds[4] = {};
            int64_t centerCopy = centerRowCol; // matches original's `local_res20 = param_4` stack copy
            if (thisPtr->ComputeFootprintBounds(bounds, buildingPtr, quadrant, &centerCopy))
            {
                int32_t rowSpan = (bounds[1] - bounds[0]) + 1;
                int32_t colSpan = (bounds[3] - bounds[2]) + 1;
                int32_t centerRow = (int32_t)(centerRowCol & 0xFFFFFFFF);
                int32_t centerCol = (int32_t)(centerRowCol >> 32);
                thisPtr->PreClearFootprint(rowSpan, colSpan, centerRow, centerCol);
            }

            return g_origAddBuildingToGrid(thisPtr, buildingPtr, quadrant, centerRowCol);
        }

        static bool SetupAddBuildingToGridHook()
        {
            if (g_origAddBuildingToGrid) return true;
            return MH_CreateHookGZ(BUILDING_GRID_ADD_BUILDING, &HookedAddBuildingToGrid, &g_origAddBuildingToGrid);
        }
    };

    static_assert(offsetof(CBuildingGrid, m_CellArrayBegin) == 0x148);
    static_assert(offsetof(CBuildingGrid, m_CellArrayEnd) == 0x150);
    static_assert(offsetof(CBuildingGrid, m_OriginRow) == 0x160);
    static_assert(offsetof(CBuildingGrid, m_OriginCol) == 0x164);
    static_assert(offsetof(CBuildingGrid, m_Width) == 0x168);
} // namespace gz
#pragma pack(pop)
