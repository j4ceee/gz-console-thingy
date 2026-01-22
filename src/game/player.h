#pragma once

#include "character.h"
#include "vector.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)

namespace gz
{
    class CPlayerAimControl
    {
    public:
        char _pad[0xB0];              // 0x000 → 0xB0
        CVector3f m_aimPos[4];        // 0xB0 → 0xD8 (4 x CVector3f, could be m_aimPos, m_lastAimPos, ..., ...)
        char _pad2[0xD8];             // 0xD8 → 0x1B0
        float m_rayCastDistance;      // 0x1B0 → 0x1B4 (500.0f)
        char _pad3[0x10];             // 0x1B4 → 0x1C4
        float m_stickyValue;          // 0x1C4 → 0x1C8 (1.0f)
        uint32_t m_aimType;           // 0x1C8 → 0x1CC (EAuto = 2)
        char _pad4[0x14];             // 0x1CC → 0x1E0
        CCharacter* m_owner;          // 0x1E0 → 0x1E8
    };

    struct PlayerCoordinateStructure
    {
        char _pad0[0x3AD8]; // 0x000 → 0x3AD8
        float worldPosX; // 0x3AD8 - base world X coordinate
        float worldPosY; // 0x3ADC - base world Y coordinate
        float worldPosZ; // 0x3AE0 - base world Z coordinate
    };

    class CPlayer
    {
    public:
        char _pad[0x58];                             // 0x000 → 0x58
        PlayerCoordinateStructure* m_coordStructure; // 0x58 → 0x60 - pointer to coordinate data structure
        char _pad1[0x88];                            // 0x60 → 0xE8
        CMatrix4f m_transform;                       // 0xE8 → 0x128 (64 bytes) - player position
        char _pad2[0x78];                            // 0x128 → 0x1A0
        CCharacter* m_character;                     // 0x1A0 → 0x1A8
        char _pad3[0x8];                             // 0x1A8 → 0x1B0
        CPlayerAimControl* m_aimControl;             // 0x1B0 → 0x1B8

        CCharacter* GetCharacter() const
        {
            // m_character points to CCharacter portion (+0x08 from base)
            // so we need to subtract 0x08 to get the actual base pointer
            if (m_character)
            {
                return reinterpret_cast<CCharacter*>(
                    reinterpret_cast<uintptr_t>(m_character) - 0x08
                );
            }
            return nullptr;
        }

        /// <summary>
        /// Gets the aim position from the player's aim control.
        /// </summary>
        CVector3f GetAimPosition() const
        {
            if (m_aimControl)
            {
                return m_aimControl->m_aimPos[0];
            }
            return CVector3f{0.0f, 0.0f, 0.0f};
        }

        float GetAimRaycastDistance() const
        {
            if (m_aimControl)
            {
                return m_aimControl->m_rayCastDistance;
            }
            return 0.0f;
        }

        CVector3f GetPositionVector() const
        {
            return CVector3f{
                m_transform.m[3].x,
                m_transform.m[3].y,
                m_transform.m[3].z
            };
        }

        CMatrix4f GetTransform() const
        {
            return m_transform;
        }

        CVector3f GetWorldCoordinates() const
        {
            if (m_coordStructure)
            {
                return CVector3f{
                    m_coordStructure->worldPosX,
                    m_coordStructure->worldPosY,
                    m_coordStructure->worldPosZ
                };
            }
            return CVector3f{0.0f, 0.0f, 0.0f};
        }

        static bool GetFPPlayerShadowEnabled()
        {
            return *reinterpret_cast<bool*>(GetAddress(VAR_FP_PLAYER_SHADOW));
        }

        static void SetFPPlayerShadowEnabled(bool enabled)
        {
            *reinterpret_cast<bool*>(GetAddress(VAR_FP_PLAYER_SHADOW)) = enabled;
        }
    };
}; // namespace gz
#pragma pack(pop)
