#pragma once

#include <cstddef>

#include "addresses.h"

#pragma pack(push, 1)
namespace gz
{
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

        char _pad5[0x654];      // 0xA4 to end
        // total size: 0x6f8


        static CMap* instance()
        {
            return *(CMap**)(GetAddress(INST_MAP));
        }
    };

    static_assert(sizeof(CMap) == 0x6F8, "CMap size mismatch!");
    static_assert(offsetof(CMap, cursorScreenX) == 0x8C, "cursorWorldX offset mismatch!");
    static_assert(offsetof(CMap, cursorScreenY) == 0x90, "cursorWorldY offset mismatch!");
    static_assert(offsetof(CMap, mapScale) == 0x9C, "mapScale offset mismatch!");
}// namespace gz
#pragma pack(pop)