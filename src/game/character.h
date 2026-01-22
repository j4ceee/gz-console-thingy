#pragma once

#pragma pack(push, 1)
#include <cmath>

#include "meow_hook/util.h"

namespace gz
{
    class CDamageable;

    class CAvatar {};

    class CCharacter
    {
    public:
        char        _pad_damageable[0x2C4]; // 0x000 → 0x2C4 (CDamageable)
        char        _pad3[0x5E4];           // 0x2C4 → 0x8A8
        int         m_originalFaction;      // 0x8A8 → 0x8AC
        char        _pad4[0x3048];          // 0x8AC → 0x38F4
        bool        m_detectable;           // 0x38F4 → 0x38F5
        char        _pad5[0xEF];            // 0x38F5 → 0x39E4
        float       m_currentGravity[2];    // 0x39E4 → 0x39EC (array of 2 floats)
        char        _pad6[0xC];             // 0x39EC → 0x39F8
        int         m_faction;              // 0x39F8 → 0x39FC
        char        _pad7[0x22C];           // 0x39FC → 0x3C28
        CAvatar*    m_avatar;               // 0x3C28 → 0x3C30

        int GetOriginalFaction() const { return m_originalFaction; }
        int GetFaction() const { return m_faction; }

        void SetFaction(int faction) { m_faction = faction; }
        void ResetFaction() { m_faction = m_originalFaction; }

        bool IsDetectable() const { return m_detectable; }
        void SetDetectable(bool detectable) { m_detectable = detectable; }

        void Revive()
        {
            meow_hook::func_call<void>(
                GetAddress(CHARACTER_REVIVE),
                this,
                true,       // call CDamageable::RestoreHealth()
                false,      // unknown
                1.0f        // health percentage
            );
        }

        void SetGhostMode(bool enable)
        {
            meow_hook::func_call<void>(
                GetAddress(CHARACTER_GHOST_MODE),
                this,
                enable
            );
        }

        CDamageable* GetDamageable() { return reinterpret_cast<CDamageable*>(this); }
    };
}; // namespace gz
#pragma pack(pop)
