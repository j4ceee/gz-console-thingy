#pragma once

#include "addresses.h"
#include "game_state.h"
#include "meow_hook/util.h"
#include <cmath>

#pragma pack(push, 1)
namespace gz
{
    inline bool playerInvulnerable = false;

    class CDamageable
    {
    public:
        char        _pad[0x130];                    // 0x000 → 0x130
        // [0x130]: unknown (used in RestoreHealth's FUN_1406c2a20 call)
        char        _pad1[0xC8];                    // 0x130 → 0x1F8
        void*       m_OnHealthDecreasedBegin;       // 0x1F8 → 0x200
        void*       m_OnHealthDecreasedEnd;         // 0x200 → 0x208
        char        _pad2[0x8];                     // 0x208 → 0x210
        int         m_MaxHealth;                    // 0x210 → 0x214
        int         m_MinHealth;                    // 0x214 → 0x218
        char        _pad3[0x78];                    // 0x218 → 0x290
        bool        m_Destroyed;                    // 0x290 → 0x291
        char        _pad4[0x3];                     // 0x291 → 0x294
        int         m_CurHealth;                    // 0x294 → 0x298
        int         m_LastHealth;                   // 0x298 → 0x29C
        char        _pad5[0x24];                    // 0x29C → 0x2C0
        uint32_t    m_InvulnerableToDamageFlags;    // 0x2C0 → 0x2C4

        int GetMaxHealth() const { return m_MaxHealth; }
        int GetHealth() const { return m_CurHealth; }
        bool IsDestroyed() const { return m_CurHealth < 1 || m_Destroyed; }
        bool IsDamaged() const { return m_CurHealth < m_MaxHealth; }

        int GetHealthInPercentage() const
        {
            if (m_MaxHealth == 0)
            {
                return 0;
            }
            return static_cast<int>(std::round(
                (static_cast<float>(m_CurHealth) / static_cast<float>(m_MaxHealth)) * 100.0f));
        }

        void SetHealthInPercentage(int percentage)
        {
            if (percentage < 0) percentage = 0;
            if (percentage > 100) percentage = 100;

            int newHealth = static_cast<int>(
                (static_cast<float>(percentage) / 100.0f) * static_cast<float>(m_MaxHealth)
            );

            SetHealth(newHealth);
        }

        /// <summary>
        /// Sets the health of the damageable object.\n
        /// <br/><b>Warning:</b> Should only be used while the entity is alive / not destroyed (check with <code>IsDestroyed()</code>).
        /// </summary>
        void SetHealth(int health)
        {
            meow_hook::func_call<void>(
                GetAddress(DAMAGEABLE_SET_HEALTH),
                this,
                health,
                nullptr, // ptr to CDamageMsg
                false // unknown
            );
        }

        void RestoreHealth()
        {
            meow_hook::func_call<void>(
                GetAddress(DAMAGEABLE_RESTORE_HEALTH),
                this,
                1.0f // 100%
            );
        }

        void SetInvulnerable(bool enabled, char type = 0)
        {
            if (type == 'p') {  // player invulnerability
                playerInvulnerable = enabled;
            }

            meow_hook::func_call<void>(
                GetAddress(DAMAGEABLE_SET_INVULNERABLE),
                this,
                enabled,
                0xffffff // damage flags (all)
            );
        }

        bool IsInvulnerable() const
        {
            return meow_hook::func_call<bool>(
                GetAddress(DAMAGEABLE_IS_INVULNERABLE),
                this,
                0xffffff // damage flags (all)
            );
        }
    };
} // namespace gz
#pragma pack(pop)
