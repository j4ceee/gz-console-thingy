#pragma once

#include <cstdint>
#include <vector>

#include "damageable_part.h"

#pragma pack(push, 1)
namespace gz
{
    class CAnimal;
    class CCharacter;

    class CAnimalCharacterComponent
    {
    public:
        void**          m_partHitreactActsBegin;    // 0x00 → 0x08
        void**          m_partHitreactActsEnd;      // 0x08 → 0x10
        void**          m_partHitreactActsCap;      // 0x10 → 0x18
        CCharacter*     m_character;                // 0x18 → 0x20
        CAnimal*        m_animalPtr;                // 0x20 → 0x28
        char            _pad1[0x4C];                // 0x28 → 0x74
        int             m_groupPercentToDestroy;    // 0x74 → 0x78
        int             m_healthPercentToDestroy;   // 0x78 → 0x7C
        float           m_turtleDamageMultiplier;   // 0x7C → 0x80
        char            _pad2[0x8];                 // 0x80 → 0x88
        uint32_t        m_lastDamageFlags;          // 0x88 → 0x8C
        char            _pad3[0x1C];                // 0x8C → 0xA8
        void**          m_allPartsBegin;            // 0xA8 → 0xB0
        void**          m_allPartsEnd;              // 0xB0 → 0xB8
        char            _pad4[0x20];                // 0xB8 → 0xD8
        void**          m_allGroupsBegin;           // 0xD8 → 0xE0
        void**          m_allGroupsEnd;             // 0xE0 → 0xE8
        char            _pad5[0x5C];                // 0xE8 → 0x144
        float           m_impactDamageBlockTimer;   // 0x144 → 0x148
        char            _pad6[0x9];                 // 0x148 → 0x151
        bool            m_unk151;                   // 0x151 → 0x152

        void SetHealthRatioOnParts(float ratio)
        {
            auto parts = GetAllParts();
            for (auto part : parts)
            {
                if (!part) continue;
                part->SetUnitHealth(ratio);
            }
        }

        std::vector<CDamageableCharacterPart*> GetAllParts()
        {
            std::vector<CDamageableCharacterPart*> parts;
            for (void** p = m_allPartsBegin; p != m_allPartsEnd; ++p)
            {
                auto part = static_cast<CDamageableCharacterPart*>(*p);
                if (part) {
                    parts.push_back(part);
                }
            }
            return parts;
        }
    };
} // namespace gz
#pragma pack(pop)