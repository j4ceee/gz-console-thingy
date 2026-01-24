#pragma once

#include "addresses.h"
#include "player.h"
#include "player_network.h"

#pragma pack(push, 1)
namespace gz
{
    class CNetworkPlayerManager
    {
    public:
        char                _pad0[0x40];        // 0x00 → 0x40
        CNetworkPlayer**    m_remotePlayers;    // 0x40 (array of pointers, not yet tested)
        char                _pad1[0x18];        // 0x48 → 0x60
        int                 m_playerCount;      // 0x60
        char                _pad2[0x4];         // 0x64 → 0x68
        CNetworkPlayer*     m_LocalPlayer;      // 0x68

        static CNetworkPlayerManager* instance()
        {
            return *(CNetworkPlayerManager**)GetAddress(INST_NETWORK_PLAYER_MANAGER);
        }

        /// <summary>
        /// Gets the local player by dereferencing through the local player structure.
        /// </summary>
        /// @return Pointer to the local player's CPlayer, or nullptr if not available.
        CPlayer* GetPlayer()
        {
            return GetNetworkPlayer()->GetPlayer();
        }

        /// <summary>
        /// Gets the local player's character directly from CNetworkPlayerManager.
        /// </summary>
        /// @return Pointer to the local player's CCharacter, or nullptr if not available.
        CCharacter* GetCharacter()
        {
            return GetNetworkPlayer()->GetCharacter();
        }

        /// <summary>
        /// Gets the local player's network component.
        /// </summary>
        /// @return Pointer to the local player's CNetworkPlayerComponent, or nullptr if not available.
        CNetworkPlayerComponent* GetPlayerNetworkComponent()
        {
            return GetNetworkPlayer()->GetNetworkComponent();
        }

        CNetworkPlayer* GetNetworkPlayer()
        {
            return m_LocalPlayer;
        }

        int GetPlayerCount() const
        {
            return m_playerCount;
        }
    };
}; // namespace gz
#pragma pack(pop)
