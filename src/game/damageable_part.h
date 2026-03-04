#pragma once

#include <algorithm>
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
    class CDamageableCharacterPart
    {
    public:
        char    _pad[0x218];    // 0x000 → 0x218
        float   m_Health;       // 0x218 → 0x21C
        float   m_MaxHealth;    // 0x21C → 0x220

        [[nodiscard]] float GetHealth() const { return m_Health; }
        [[nodiscard]] float GetMaxHealth() const { return m_MaxHealth; }

        void SetHealth(float health)
        {
            m_Health = std::clamp(health, 0.0f, m_MaxHealth);
        }

        void SetUnitHealth(float ratio)
        {
            // clamp ratio to 0-1
            ratio = std::clamp(ratio, 0.0f, 1.0f);

            meow_hook::func_call<int>(
                GetAddress(DAMAGEABLE_PART_SET_UNIT_HEALTH),
                this,
                ratio
            );
        }
    };
} // namespace gz
#pragma pack(pop)
