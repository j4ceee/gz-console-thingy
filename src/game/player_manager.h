#pragma once

#include "addresses.h"
#include "player.h"
#include "player_network.h"

#pragma pack(push, 1)
namespace gz
{
struct LocalPlayerStructure
{
    char _pad0[0x1C];                           //   0x0 → 0x1C  (28 bytes)
    char m_profileName[18];                     //  0x1C → 0x2E  (18 bytes)
    char _pad1[0x112];                          //  0x2E → 0x140 (274 bytes)
    CPlayer* m_playerPtr;                       // 0x140 → 0x148 (8 bytes)
    char _pad2[0x8];                            // 0x148 → 0x150
    CCharacter* m_characterPtr;                 // 0x150 → 0x158 (base pointer, actual CCharacter is at +0x08)
    char _pad3[0x18];                           // 0x158 → 0x170
    CNetworkPlayerComponent* m_networkCompPtr;  // 0x170 → 0x178 (8 bytes)
};

class CNetworkPlayerManager
{
public:
    char _pad0[0x40];                               // 0x00 → 0x40
    LocalPlayerStructure** m_remotePlayers;         // 0x40 (array of pointers, not yet tested)
    char _pad1[0x18];                               // 0x48 → 0x60
    int m_playerCount;                              // 0x60
    char _pad2[0x4];                                // 0x64 → 0x68
    LocalPlayerStructure* m_localPlayerStructure;   // 0x68

    static CNetworkPlayerManager& instance()
    {
        return **(CNetworkPlayerManager**)GetAddress(INST_NETWORK_PLAYER_MANAGER);
    }

    /// <summary>
    /// Gets the local player by dereferencing through the local player structure.
    /// </summary>
    /// @return Pointer to the local player's CPlayer, or nullptr if not available.
    static CPlayer* GetLocalPlayer()
    {
        auto& inst = instance();
        if (inst.m_localPlayerStructure) {
            return inst.m_localPlayerStructure->m_playerPtr;
        }
        return nullptr;
    }

    /// <summary>
    /// Gets the local player's character directly from CNetworkPlayerManager.\n
    /// Access over CPlayer preferred.
    /// </summary>
    /// @return Pointer to the local player's CCharacter, or nullptr if not available.
    static CCharacter* GetLocalPlayerCharacter()
    {
        auto& inst = instance();
        if (inst.m_localPlayerStructure) {
            return inst.m_localPlayerStructure->m_characterPtr + 0x08; // actual character is at +0x08
        }
        return nullptr;
    }

    /// <summary>
    /// Gets the local player's network component.
    /// </summary>
    /// @return Pointer to the local player's CNetworkPlayerComponent, or nullptr if not available.
    static CNetworkPlayerComponent* GetLocalPlayerNetworkComponent()
    {
        auto& inst = instance();
        if (inst.m_localPlayerStructure) {
            return inst.m_localPlayerStructure->m_networkCompPtr;
        }
        return nullptr;
    }

    /// <summary>
    /// Gets the local player's profile name as a std::string.
    /// </summary>
    /// @return Local player's profile name, or empty string if not available.
    std::string GetLocalPlayerProfileName() const
    {
        if (m_localPlayerStructure) {
            return std::string(m_localPlayerStructure->m_profileName);
        }
        return "";
    }

    int GetPlayerCount() const
    {
        return m_playerCount;
    }
};
}; // namespace gz
#pragma pack(pop)
