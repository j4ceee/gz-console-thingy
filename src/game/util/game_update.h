#pragma once

#include "game/damageable.h"
#include "game/game_state.h"
#include "game/player_network_manager.h"
#include "game/vehicle.h"
#include "game/vehicle_manager.h"
#include "imgui/ui.h"

#pragma pack(push, 1)
namespace gz::Utils
{
    /// <summary>
    /// Function that runs every game update tick.\n
    /// Any logic that needs to be updated on each tick should go here.
    /// </summary>
    static void OnGameUpdate()
    {
        // custom logic that needs to run every game update
        if (GameState::IsInGame())
        {
            UI* ui = UI::Get();
            ConsoleSettings& settings = ui->GetSettings();

            if (CNetworkPlayerManager* networkPlayerManager = CNetworkPlayerManager::instance())
            {
                CCharacter* playerCharacter = networkPlayerManager->GetCharacter();

                // update player invulnerability status
                CDamageable* playerDamageable = playerCharacter->GetDamageable();
                if (playerInvulnerable && !playerDamageable->IsInvulnerable())
                {
                    playerDamageable->SetInvulnerable(true);
                }
            }

            // update vehicle invulnerability status
            if (CVehicleManager* vehicleManager = CVehicleManager::instance())
            {
                auto* vehicle = vehicleManager->GetPlayerVehicle();
                if (vehicle)
                {
                    CDamageable* vehicleDamageable = vehicle->GetDamageable();
                    if (settings.enableInfiniteBikeHealth && !vehicleDamageable->IsInvulnerable())
                    {
                        vehicleDamageable->SetInvulnerable(true);
                    }
                }
            }
        }
    }
} // namespace gz::Utils
#pragma pack(pop)