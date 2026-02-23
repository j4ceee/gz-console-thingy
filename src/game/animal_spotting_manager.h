#pragma once

#include <memory>
#include <vector>

#include "animal.h"
#include "data_types.h"

#pragma pack(push, 1)
namespace gz
{
    class CAnimalSpottingManager;

    struct SSpottedAnimal
    {
        uint32_t        m_Timer;                // 0x00 → 0x04
        float           m_XpMultiplier;         // 0x04 → 0x08
        float           m_DamageMultiplier;     // 0x08 → 0x0C
        char            _pad[0x4];              // 0x0C → 0x10
        CAnimal*        m_Animal;               // 0x10 → 0x18
        BasicSharedPtr  m_Character;            // 0x18 → 0x28 (16 bytes)
    };

    using RequestFindTargetFunc = void(*)(void* casm, void* p2);
    inline RequestFindTargetFunc g_findTarget = nullptr;

    class CAnimalSpottingManager
    {
    public:
        char        _pad0[0x4];             // 0x00 → 0x04
        float       m_SpottingScreenRadius; // 0x04 → 0x08
        float       m_ScreenSizeThreshold;  // 0x08 → 0x0C
        float       m_VisibilityThreshold;  // 0x0C → 0x10
        CAnimal*    m_TargetAnimal;         // 0x10 → 0x18
        CAnimal*    m_LastTargetAnimal;     // 0x18 → 0x20
        BasicVector m_SpottedAnimals;       // 0x20 → 0x38
        char        _pad1[0x10];            // 0x38 → 0x48
        float       m_TargetTimer;          // 0x48 → 0x4C
        float       m_TargetAnimalDistance; // 0x4C → 0x50

        size_t GetSpottedAnimalCount() const
        {
            auto& vec = *reinterpret_cast<const std::vector<SSpottedAnimal>*>(&m_SpottedAnimals);
            return vec.size();
        }

        SSpottedAnimal* GetSpottedAnimal(size_t index)
        {
            auto& vec = *reinterpret_cast<std::vector<SSpottedAnimal>*>(&m_SpottedAnimals);
            return (index < vec.size()) ? &vec[index] : nullptr;
        }

        CAnimal* GetTargetAnimal() const
        {
            return m_TargetAnimal;
        }

        CAnimal* GetLastTargetAnimal() const
        {
            return m_LastTargetAnimal;
        }

        float GetTargetAnimalDistance() const
        {
            return m_TargetAnimalDistance;
        }
    };
} // namespace gz
#pragma pack(pop)