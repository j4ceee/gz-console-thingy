#pragma once

#include <algorithm>

#include "animal_type.h"
#include "character.h"
#include "data_types.h"

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
        char            _pad3[0x20];            // 0xA0 → 0xC0
        CAnimalType*    m_AnimalType;           // 0xC0 → 0xC8
        char            _pad4[0x50];            // 0xC8 → 0x118
        float           m_MaxOrganGroupHealth;  // 0x118 → 0x11C
        float           m_OrganGroupHealth;     // 0x11C → 0x120
        float           m_MinKillDamage;        // 0x120 → 0x124
        char            _pad5[0xBC];            // 0x124 → 0x1E0
        uint32_t        m_VisualVariationSeed;  // 0x1E0 → 0x1E4

        CCharacter* GetSpawnedCharacter() const
        {
            return *reinterpret_cast<CCharacter**>(
                const_cast<char*>(reinterpret_cast<const char*>(&m_SpawnedCharacter))
            );
        }

        CAnimalType* GetAnimalType() const
        {
            return m_AnimalType;
        }

        float GetHealthPercent() const
        {
            if (m_MinKillDamage <= 0.0f) return 0.0f;
            float hp = (m_OrganGroupHealth - (m_MaxOrganGroupHealth - m_MinKillDamage)) / m_MinKillDamage;
            return std::clamp(hp * 100.0f, 0.0f, 100.0f);
        }
    };
} // namespace gz
#pragma pack(pop)