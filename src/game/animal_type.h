#pragma once

#include "data_types.h"

#pragma pack(push, 1)
namespace gz
{
    class CAnimal;

    class CAnimalGroup
    {
    public:
        char        _pad0[0x382];           // 0x000 → 0x382
        uint8_t     m_DisableSimulation;    // 0x382 → 0x383
        char        _pad1[0x15];            // 0x383 → 0x398
        CAnimal*    m_Leader;               // 0x398 → 0x3A0
        BasicVector m_Animals;              // 0x3A0 → 0x3B8
    };
    static_assert(offsetof(CAnimalGroup, m_DisableSimulation) == 0x382);
    static_assert(offsetof(CAnimalGroup, m_Leader) == 0x398);
    static_assert(offsetof(CAnimalGroup, m_Animals) == 0x3A0);

    class CAnimalType
    {
    public:
        uint32_t    m_AnimalTypeNameHash;           // 0x00 → 0x04
        uint32_t    m_AnimalNameLocalizationKey;    // 0x04 → 0x08
        uint32_t    m_Category;                     // 0x08 → 0x0C
        char        _pad0[0x04];                    // 0x0C → 0x10
        GameString  m_Name;                         // 0x10 → 0x30 (32 bytes)
        GameString  m_SpawnTag;                     // 0x30 → 0x50 (32 bytes)
        GameString  m_MachineClass;                 // 0x50 → 0x70 (32 bytes)
        float       m_RoamSpeed;                    // 0x70 → 0x74
        float       m_TrotSpeed;                    // 0x74 → 0x78
        char        _pad1[0x128];                   // 0x78 → 0x1A0
        int32_t     m_Difficulty;                   // 0x1A0 → 0x1A4
        char        _pad2[0x04];                    // 0x1A4 → 0x1A8
        GameString  m_Designator;                   // 0x1A8 → 0x1C8 (32 bytes)

        [[nodiscard]] const char* GetName() const
        {
            return m_Name.c_str();
        }

        [[nodiscard]] const char* GetMachineClass() const
        {
            return m_MachineClass.c_str();
        }

        [[nodiscard]] const char* GetSpawnTag() const
        {
            return m_SpawnTag.c_str();
        }

        [[nodiscard]] const char* GetDesignator() const
        {
            return m_Designator.c_str();
        }

    };
} // namespace gz
#pragma pack(pop)