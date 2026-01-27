#pragma once

#include "character.h"
#include "vector.h"

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

    class CPlayer
    {
    public:
        // CPlayer-specific data (before CAvatar)
        char _pad_player[0x10];             //  0x00 → 0x10
        // CAvatar starts here
        char _pad[0x58];                    //  0x00 → 0x68
        CCharacter* m_character;            //  0x68 → 0x70
        char _pad1[0x88];                   //  0x70 → 0xF8
        CMatrix4f m_transform;              //  0xF8 → 0x138 (64 bytes) - player position
        char _pad2[0x78];                   // 0x138 → 0x1B0
        CCharacter* m_character2;           // 0x1B0 → 0x1B8 (same as m_character but +0x08 from base)
        char _pad3[0x8];                    // 0x1B8 → 0x1C0
        CPlayerAimControl* m_aimControl;    // 0x1C0 → 0x1C8

        CCharacter* GetCharacter() const
        {
            if (m_character)
            {
                return m_character;
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
