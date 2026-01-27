#pragma once

#include "hook_helpers.h"
#include "remote_controller.h"
#include "vector.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
    class CDamageable;

    class CAvatar {};

    class CCharacter
    {
    public:
        char                _pad_damageable[0x2C4]; // 0x000 → 0x2C4 (CDamageable)
        char                _pad3[0x5E4];           // 0x2C4 → 0x8A8
        int                 m_originalFaction;      // 0x8A8 → 0x8AC
        char                _pad4[0x2C];            // 0x8AC → 0x8D8
        void**              m_interactionData;      // 0x8D8 → 0x8E0 - pointer to interaction data
        char                _pad5[0x2B10];          // 0x8AC → 0x33F0
        CRemoteController*  m_remoteController;     // 0x33F0 → 0x33F8
        char                _pad6[0x11];            // 0x33F8 → 0x3409
        uint8_t             m_controlFlag;          // 0x3409 → 0x340A
        char                _pad7[0x4EA];           // 0x340A → 0x38F4
        bool                m_detectable;           // 0x38F4 → 0x38F5
        char                _pad8[0xEF];            // 0x38F5 → 0x39E4
        float               m_currentGravity[2];    // 0x39E4 → 0x39EC (array of 2 floats)
        char                _pad9[0xC];             // 0x39EC → 0x39F8
        int                 m_faction;              // 0x39F8 → 0x39FC
        char                _pad10[0xDC];           // 0x39FC → 0x3AD8
        float               worldPosX;              // 0x3AD8 → 0x3ADC - base world X coordinate
        float               worldPosY;              // 0x3ADC → 0x3AE0 - base world Y coordinate
        float               worldPosZ;              // 0x3AE0 → 0x3AE4 - base world Z coordinate
        char                _pad11[0x144];          // 0x3AE4 → 0x3C28
        CAvatar*            m_avatar;               // 0x3C28 → 0x3C30

        int GetOriginalFaction() const { return m_originalFaction; }
        int GetFaction() const { return m_faction; }

        void SetFaction(int faction) { m_faction = faction; }
        void ResetFaction() { m_faction = m_originalFaction; }

        bool IsDetectable() const { return m_detectable; }
        void SetDetectable(bool detectable) { m_detectable = detectable; }

        CVector3f GetPosition() const
        {
            return CVector3f{worldPosX, worldPosY, worldPosZ};
        }

        float GetDistanceTo(const CCharacter* other) const
        {
            if (!other) return -1.0f;

            CVector3f myPos = GetPosition();
            CVector3f otherPos = other->GetPosition();

            float dx = myPos.x - otherPos.x;
            float dy = myPos.y - otherPos.y;
            float dz = myPos.z - otherPos.z;

            return sqrtf(dx*dx + dy*dy + dz*dz);
        }

        float GetInteractionRadius() const
        {
            // Double-pointer dereference to get radius
            if (!m_interactionData || !*m_interactionData) {
                return 0.0f;
            }
            float* radiusPtr = reinterpret_cast<float*>(*m_interactionData);
            return *radiusPtr;  // Returns 150.0 for this machine
        }

        bool IsWithinInteractionRange(const CCharacter* other) const
        {
            if (!other) return false;

            float radius = other->GetInteractionRadius();
            if (radius <= 0.0f) return false;

            float distance = GetDistanceTo(other);

            return distance <= (radius * 0.5f); // within half the interaction radius
        }

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

        CRemoteController* GetRemoteController() const
        {
            return m_remoteController;
        }
    };
}; // namespace gz
#pragma pack(pop)
