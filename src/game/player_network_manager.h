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
        CNetworkPlayer*     m_remotePlayers[4]; // 0x40 → 0x60
        int                 m_playerCount;      // 0x60
        char                _pad2[0x4];         // 0x64 → 0x68
        CNetworkPlayer*     m_LocalPlayer;      // 0x68

        [[nodiscard]] static CNetworkPlayerManager* instance()
        {
            return *(CNetworkPlayerManager**)GetAddress(INST_NETWORK_PLAYER_MANAGER);
        }

        /// <summary>
        /// Gets the local player by dereferencing through the local player structure.
        /// </summary>
        /// @return Pointer to the local player's CPlayer, or nullptr if not available.
        [[nodiscard]] CPlayer* GetPlayer()
        {
            return GetNetworkPlayer()->GetPlayer();
        }

        /// <summary>
        /// Gets the local player's character directly from CNetworkPlayerManager.
        /// </summary>
        /// @return Pointer to the local player's CCharacter, or nullptr if not available.
        [[nodiscard]] CCharacter* GetCharacter()
        {
            return GetNetworkPlayer()->GetCharacter();
        }

        /// <summary>
        /// Gets the local player's network component.
        /// </summary>
        /// @return Pointer to the local player's CNetworkPlayerComponent, or nullptr if not available.
        [[nodiscard]] CNetworkPlayerComponent* GetPlayerNetworkComponent()
        {
            return GetNetworkPlayer()->GetNetworkComponent();
        }

        /// <summary>
        /// Gets the local player's network player instance.
        /// </summary>
        /// @return Pointer to the local player's CNetworkPlayer, or nullptr if not available.
        [[nodiscard]] CNetworkPlayer* GetNetworkPlayer()
        {
            return m_LocalPlayer;
        }

        [[nodiscard]] int GetPlayerCount() const
        {
            return m_playerCount;
        }

        // get other players by index (not yet tested, may not be correct)
        [[nodiscard]] CNetworkPlayer* GetRemotePlayer(int index)
        {
            if (index < 0 || index >= 4) {
                return nullptr; // index must be between 0 and 3
            }
            return m_remotePlayers[index];
        }
    };
} // namespace gz
#pragma pack(pop)
