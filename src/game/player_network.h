#pragma once

#include "player_skills.h"

#pragma pack(push, 1)

namespace gz
{
class CNetworkPlayerComponent
{
public:
    char            _pad[0x28C];
    CPlayerSkills*  m_playerSkills;     // +0x28C (pointer to skills object)
    char            _pad2[0x374];       // 0x28C + 0x8 (ptr) = 0x294, then 0x608 - 0x294 = 0x374
    bool            m_infiniteStamina;  // +0x608

    bool GetInfiniteStamina() const { return m_infiniteStamina; }
    void SetInfiniteStamina(bool enabled) { m_infiniteStamina = enabled; }
};
}
#pragma pack(pop)
