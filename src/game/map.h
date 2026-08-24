#pragma once

#include <cstddef>

#include "addresses.h"
#include "ui_base.h"

#pragma pack(push, 1)
namespace gz
{
    struct SMapIcon
    {
        char        _pad0[0x18];
        CVector3f   m_Position;     // 0x18
        char        _pad1[0x0C];    // 0x24 -> 0x30
        int64_t     m_IdHash;       // 0x30
        char        _pad2[0x16];    // 0x38 -> 0x4E
        uint8_t     m_CanFastTravelTo; // 0x4E
        char        _pad3[0x09];    // 0x4F -> 0x58
    };
    static_assert(sizeof(SMapIcon) == 0x58);
    static_assert(offsetof(SMapIcon, m_Position) == 0x18);
    static_assert(offsetof(SMapIcon, m_IdHash) == 0x30);
    static_assert(offsetof(SMapIcon, m_CanFastTravelTo) == 0x4E);

    class CMap;

    inline bool g_fastTravelAnywhereEnabled = true;

    using IsAllowedInRegionFunc_t = bool(*)(CMap*, CVector3f*);
    inline IsAllowedInRegionFunc_t g_origIsAllowedInRegion = nullptr;
    using FastTravelFunc_t = void(*)(CMap*, void*);
    inline FastTravelFunc_t g_origFastTravel = nullptr;
    using OnManageInputIconsFunc_t = void(*)(CMap*, uint8_t*, void*);
    inline OnManageInputIconsFunc_t g_origOnManageInputIcons = nullptr;

    class CMap
    {
    public:
        void* vtable;           // 0x00
        char _pad0[8];          // 0x08
        void* vtable2;          // 0x10 (second vtable pointer)
        char _pad1[0x48];       // 0x18

        char _pad2[0x10];       // 0x60
        uint64_t field_70;      // 0x70
        uint32_t field_78;      // 0x78

        float mapCenterX;       // 0x7C
        float mapCenterY;       // 0x80
        float mapCenterZ;       // 0x84
        char _pad3[4];          // 0x88

        float cursorScreenX;    // 0x8C - screen X (0,0 at top-left corner)
        float cursorScreenY;    // 0x90 - screen Y (0,0 at top-left corner)
        char _pad4[8];          // 0x98

        float mapScale;         // 0x9C (initialized to 24.0f)
        float mapScale2;        // 0xA0 (initialized to 24.0f)

        char        _pad5a[0x7C];       // 0xA4 -> 0x120
        SMapIcon*   m_IconArrayBegin;   // 0x120
        SMapIcon*   m_IconArrayEnd;     // 0x128
        char        _pad5b[0x5C8];      // 0x130 -> 0x6F8
        // total size: 0x6f8

        static CMap* instance()
        {
            return *(CMap**)(GetAddress(INST_MAP));
        }


        static CVector2f WorldToMapPosition(float worldX, float worldZ)
        {
            CVector2f mapPos{};
            meow_hook::func_call<void>(GetAddress(WORLD_TO_MAP_COORDS), worldX, worldZ, &mapPos);
            return mapPos;
        }


        // -- patch dlc map check
        static bool HookedIsAllowedInRegion(CMap* thisPtr, CVector3f* position)
        {
            return true;
        }

        static bool SetupIsAllowedInRegionHook()
        {
            if (g_origIsAllowedInRegion) return true;
            return MH_CreateHookGZ(MAP_ALLOWED_IN_REGION, &HookedIsAllowedInRegion, &g_origIsAllowedInRegion);
        }


        // -- patch map icon fast travel
        static void HookedOnManageInputIcons(CMap* thisPtr, uint8_t* iconInfo, void* param3)
        {
            if (g_fastTravelAnywhereEnabled && iconInfo)
            {
                iconInfo[8] = 1; // force "CanBeFastTravelledTo" true before the real function reads it
            }
            g_origOnManageInputIcons(thisPtr, iconInfo, param3);
        }

        static bool SetupOnManageInputIconsHook()
        {
            if (g_origOnManageInputIcons) return true;
            return MH_CreateHookGZ(MAP_ON_MANAGE_INPUT_ICONS, &HookedOnManageInputIcons, &g_origOnManageInputIcons);
        }


        // -- patch fast travel check
        static void HookedFastTravel(CMap* thisPtr, void* destination)
        {
            uint8_t* patchedFlag = nullptr;
            uint8_t savedValue = 0;

            if (g_fastTravelAnywhereEnabled)
            {
                int64_t targetHash = ((CUIBase*)destination)->GetUint64Field();
                if (targetHash != -1)
                {
                    for (auto* icon = thisPtr->m_IconArrayBegin; icon != thisPtr->m_IconArrayEnd; icon++)
                    {
                        if (icon->m_IdHash == targetHash)
                        {
                            patchedFlag = &icon->m_CanFastTravelTo;
                            savedValue = *patchedFlag;
                            *patchedFlag = 1;
                            break;
                        }
                    }
                }
            }

            g_origFastTravel(thisPtr, destination);

            if (patchedFlag)
            {
                *patchedFlag = savedValue; // restore -> don't permanently alter the icon's real state
            }
        }

        static bool SetupFastTravelHook()
        {
            if (g_origFastTravel) return true;
            return MH_CreateHookGZ(MAP_FAST_TRAVEL, &HookedFastTravel, &g_origFastTravel);
        }
    };
    static_assert(sizeof(CMap) == 0x6F8, "CMap size mismatch!");
    static_assert(offsetof(CMap, cursorScreenX) == 0x8C, "cursorWorldX offset mismatch!");
    static_assert(offsetof(CMap, cursorScreenY) == 0x90, "cursorWorldY offset mismatch!");
    static_assert(offsetof(CMap, mapScale) == 0x9C, "mapScale offset mismatch!");
} // namespace gz
#pragma pack(pop)