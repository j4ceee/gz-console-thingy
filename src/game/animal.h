#pragma once

#include "character.h"
#include "vector.h"

#pragma pack(push, 1)
namespace gz
{
    class CAnimal
    {
    public:
        char            _pad0[0x38];            // 0x00 → 0x38
        uint32_t        m_NetworkComponentId;   // 0x38 → 0x3C
        char            _pad1[0x54];            // 0x3C → 0x90
        BasicSharedPtr  m_SpawnedCharacter;     // 0x90 → 0xA0
    };
} // namespace gz
#pragma pack(pop)