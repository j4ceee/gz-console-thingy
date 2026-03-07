#pragma once

#include "animation_control.h"
#include "remote_controller.h"
#include "data_types.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
    class CDeepWaterHandling;
    class CAnimalCharacterComponent;
    class CDamageable;
    class CAvatar {};

    struct CObjectBlackboard
    {
        void*       m_KeyInfos;         // +0x00
        uint16_t    m_KeyInfoCapacity;  // +0x08
        uint8_t     _pad1[6];           // +0x0A
        void*       m_Data;             // +0x10
        uint16_t    m_DataCapacity;     // +0x18
        uint16_t    m_CurrentOffset;    // +0x1A
        uint8_t     _pad2[4];           // +0x1C
        void*       m_NetCallback;      // +0x20
        void*       m_NetUserData;      // +0x28
        void*       m_BbLock;           // +0x30
    };
    static_assert(sizeof(CObjectBlackboard) == 0x38);

    class CPfxCharacterInstance
    {
    public:
        char    _pad[0xC0];         // 0x00 → 0xC0
        float   m_GroundDistance;   // 0xC0 → 0xC4
        char    _pad2[0x38];        // 0xC4 → 0xFC
        float   m_Gravity;          // 0xFC → 0x100

        [[nodiscard]] float GetGravity() const { return m_Gravity; }
        [[nodiscard]] float GetGroundDistance() const { return m_GroundDistance; }

        [[nodiscard]] float GetGravityInGs() const
        {
            return m_Gravity / -9.810f;
        }

        void SetGravityInGs(float gs)
        {
            m_Gravity = gs * -9.810f;
        }

        void ResetGravityForPlayer()
        {
            m_Gravity = -21; // default gravity value for the player character
        }
    };

    class CCharacter
    {
    public:
        char                        _pad_damageable[0x2C4]; // 0x0000 → 0x02C4 (CDamageable)
        char                        _pad1[0x5E4];           // 0x02C4 → 0x08A8
        int                         m_originalFaction;      // 0x08A8 → 0x08AC
        char                        _pad2[0x2C];            // 0x08AC → 0x08D8
        void**                      m_interactionData;      // 0x08D8 → 0x08E0 - pointer to interaction data
        char                        _pad3[0x988];           // 0x08E0 → 0x1268
        CAnimatedModel              m_animatedModel;        // 0x1268 → 0x1390
        char                        _pad4[0x2010];          // 0x1270 → 0x33A0
        CPfxCharacterInstance*      m_pfxInstance;          // 0x33A0 → 0x33A8
        char                        _pad5[0x8];             // 0x33A8 → 0x33B0
        CObjectBlackboard           m_Blackboard;           // 0x33B0 → 0x33E8
        char                        _pad6[0x8];             // 0x33E8 → 0x33F0
        CRemoteController*          m_remoteController;     // 0x33F0 → 0x33F8
        char                        _pad7[0x10];            // 0x33F8 → 0x3408
        bool                        m_unlimitedAmmo;        // 0x3408 → 0x3409 (for player it still consumes inventory ammo)
        uint8_t                     m_controlFlag;          // 0x3409 → 0x340A
        char                        _pad8[0x4EA];           // 0x340A → 0x38F4
        bool                        m_detectable;           // 0x38F4 → 0x38F5
        char                        _pad9[0xEF];            // 0x38F5 → 0x39E4
        CVector2f                   m_currentGravity;       // 0x39E4 → 0x39EC (array of 2 floats)
        char                        _pad10[0xC];            // 0x39EC → 0x39F8
        int                         m_faction;              // 0x39F8 → 0x39FC
        char                        _pad11[0xDC];           // 0x39FC → 0x3AD8
        float                       worldPosX;              // 0x3AD8 → 0x3ADC - base world X coordinate
        float                       worldPosY;              // 0x3ADC → 0x3AE0 - base world Y coordinate
        float                       worldPosZ;              // 0x3AE0 → 0x3AE4 - base world Z coordinate
        char                        _pad12[0x144];          // 0x3AE4 → 0x3C28
        CAvatar*                    m_avatar;               // 0x3C28 → 0x3C30
        char                        _pad13[0x758];          // 0x3C30 → 0x4388
        CAnimalCharacterComponent*  m_animalComponent;      // 0x4388 → 0x4390
        char                        _pad14[0x10];           // 0x4390 → 0x43A0
        CDeepWaterHandling*         m_deepWaterHandling;    // 0x43A0 → 0x43A8

        [[nodiscard]] CDamageable* GetDamageable()
        {
            return reinterpret_cast<CDamageable*>(this);
        }

        [[nodiscard]] CPfxCharacterInstance* GetPfxInstance()
        {
            return m_pfxInstance;
        }

        /// <summary>
        /// Gets the animal component of the character, if it has one.
        /// </summary>
        /// <returns>A pointer to the animal component, or nullptr if the character does not have one.</returns>
        [[nodiscard]] CAnimalCharacterComponent* GetAnimalComponent()
        {
            return m_animalComponent;
        }

        [[nodiscard]] CAnimatedModel& GetAnimatedModel()
        {
            return m_animatedModel;
        }

        [[nodiscard]] CRemoteController* GetRemoteController() const
        {
            return m_remoteController;
        }

        [[nodiscard]] int GetOriginalFaction() const { return m_originalFaction; }
        [[nodiscard]] int GetFaction() const { return m_faction; }

        void SetFaction(int faction) { m_faction = faction; }
        void ResetFaction() { m_faction = m_originalFaction; }

        [[nodiscard]] bool IsDetectable() const { return m_detectable; }
        void SetDetectable(bool detectable) { m_detectable = detectable; }

        [[nodiscard]] CVector3f GetPosition() const
        {
            return CVector3f{worldPosX, worldPosY, worldPosZ};
        }

        [[nodiscard]] bool IsSoviet() const { return m_originalFaction == 5; }
        [[nodiscard]] bool IsFNIX() const { return m_originalFaction == 2; }
        [[nodiscard]] bool IsResistance() const { return m_originalFaction == 0; }

        /// <summary>
        /// Get the distance to another character in meters.
        /// </summary>
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

        [[nodiscard]] float GetInteractionRadius() const
        {
            // double-pointer dereference to get radius
            if (!m_interactionData || !*m_interactionData) {
                return 0.0f;
            }
            const auto* radiusPtr = static_cast<float*>(*m_interactionData);
            return *radiusPtr;
        }

        /// <summary>
        /// Check if the character is within interaction range of another character.\n
        /// e.g. check if playerChar (this) is within interaction range of a machineChar (other)
        /// </summary>
        bool IsWithinInteractionRangeOf(const CCharacter* other) const
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

        [[nodiscard]] bool IsControllingEntity() const
        {
            return (m_controlFlag & 0x40) != 0;
        }

        /// <summary>
        /// Sets the visibility of the third person character body
        /// </summary>
        void SetThirdPersonBodyVisible(bool visible)
        {
            meow_hook::func_call<void>(
                GetAddress(FUNC_SET_BLACKBOARD_INT_GET_WRAPPER),
                &m_Blackboard,
                0x41df1e71,
                visible ? 1 : 0,
                0,
                0
            );
        }
    };
    static_assert(offsetof(CCharacter, m_Blackboard) == 0x33B0);
    static_assert(offsetof(CCharacter, m_pfxInstance) == 0x33A0);
    static_assert(offsetof(CCharacter, m_animalComponent) == 0x4388);
} // namespace gz
#pragma pack(pop)
