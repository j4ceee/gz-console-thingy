#pragma once

#include "player_skills.h"

#pragma pack(push, 1)

namespace gz
{
    class CNetworkPlayerComponent
    {
    public:
        char _pad[0x28C];               // 0x000 → 0x28C
        CPlayerSkills* m_playerSkills;  // +0x28C (pointer to skills object)
        char _pad2[0x374];              // 0x294 → 0x608
        bool m_infiniteStamina;         // +0x608

        bool GetInfiniteStamina() const { return m_infiniteStamina; }
        void SetInfiniteStamina(bool enabled) { m_infiniteStamina = enabled; }
    };


    class CNetworkPlayer
    {
    public:
        char _pad0[0x1C];                           //   0x0 → 0x1C  (28 bytes)
        char m_profileName[18];                     //  0x1C → 0x2E  (18 bytes)
        char _pad1[0xF2];                           //  0x2E → 0x120 (242 bytes)
        uint8_t m_peerID;                           // 0x120 → 0x121 (1 byte)
        char _pad1b[0x7];                           // 0x121 → 0x128 (7 bytes)
        uint64_t m_guid;                            // 0x128 → 0x130 (8 bytes)
        char _pad2[0x10];                           // 0x130 → 0x140 (16 bytes)
        CPlayer* m_playerPtr;                       // 0x140 → 0x148 (8 bytes)
        char _pad3[0x8];                            // 0x148 → 0x150
        CCharacter* m_characterPtr;                 // 0x150 → 0x158 (same as CPlayer->m_character)
        char _pad4[0x8];                            // 0x158 → 0x160
        uint32_t m_characterNetworkId;              // 0x160 → 0x164 (4 bytes)
        char _pad5[0xC];                            // 0x164 → 0x170 (12 bytes)
        CNetworkPlayerComponent* m_networkCompPtr;  // 0x170 → 0x178 (8 bytes)
        char _pad6[0x8];                            // 0x178 → 0x180
        void* m_mutex;                              // 0x180 → 0x188 (8 bytes)

        CNetworkPlayerComponent* GetNetworkComponent() const
        {
            return m_networkCompPtr;
        }

        CPlayer* GetPlayer() const
        {
            // m_playerPtr points to CAvatar portion (+0x10 from base)
            // so we need to subtract 0x10 to get the actual base CPlayer pointer
            return reinterpret_cast<CPlayer*>(
                reinterpret_cast<uintptr_t>(m_playerPtr) - 0x10
            );
        }

        CCharacter* GetCharacter() const
        {
            return m_characterPtr;
        }

        std::string GetProfileName() const
        {
            return std::string(m_profileName, strnlen_s(m_profileName, sizeof(m_profileName)));
        }
    };
} // namespace gz
#pragma pack(pop)
