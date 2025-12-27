#pragma once

#include "character.h"
#include "vector.h"

#pragma pack(push, 1)

namespace gz
{
class CPlayerAimControl
{
public:
    char      _pad[0xB0];           // 0x000 → 0xB0
    CVector3f m_aimPos;             // 0xB0 → 0xBC (12 bytes)
};

class CPlayer
{
struct PlayerCoordinateStructure
{
    char      _pad0[0x3AD8];    // 0x000 → 0x3AD8
    float     worldPosX;        // 0x3AD8 - base world X coordinate
    float     worldPosY;        // 0x3ADC - base world Y coordinate
    float     worldPosZ;        // 0x3AE0 - base world Z coordinate
};

public:
    char                        _pad[0x58];         // 0x000 → 0x58
    PlayerCoordinateStructure*  m_coordStructure;   // 0x58 → 0x60 - pointer to coordinate data structure
    char                        _pad1[0x88];        // 0x60 → 0xE8
    CMatrix4f                   m_transform;        // 0xE8 → 0x128 (64 bytes) - player position
    char                        _pad2[0x78];        // 0x128 → 0x1A0
    CCharacter*                 m_character;        // 0x1A0 → 0x1A8
    char                        _pad3[0x8];         // 0x1A8 → 0x1B0
    CPlayerAimControl*          m_aimControl;       // 0x1B0 → 0x1B8

    CCharacter* GetCharacter() const
    {
        return m_character;
    }

    /// <summary>
    /// Gets the aim position from the player's aim control.
    /// </summary>
    CVector3f GetAimPosition() const
    {
        if (m_aimControl) {
            return m_aimControl->m_aimPos;
        }
        return CVector3f{0.0f, 0.0f, 0.0f};
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
        if (m_coordStructure) {
            return CVector3f{
                m_coordStructure->worldPosX,
                m_coordStructure->worldPosY,
                m_coordStructure->worldPosZ
            };
        }
        return CVector3f{0.0f, 0.0f, 0.0f};
    }
};
}; // namespace gz
#pragma pack(pop)
