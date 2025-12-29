#include "../ui.h"
#include "../fonts/IconsMaterialDesign.h"
#include "game/player_manager.h"
#include "patches/building_patches.h"
#include "patches/health_patch.h"
#include "patches/resource_patch.h"

#include <imgui.h>

#include "game/player_info.h"
#include "game/vehicle.h"
#include "game/vehicle_manager.h"
#include "game/weapon_consumption.h"
#include "patches/detection_patch.h"
#include "patches/ui_patches.h"
#include "patches/vehicle_patches.h"

namespace gz::UITabs
{
void RenderPlayerTab()
{
    UI* ui = UI::Get();
    ConsoleSettings& settings = ui->GetSettings();
    auto* character = CNetworkPlayerManager::GetLocalPlayer()->GetCharacter();

    // -- HEALTH --
    if (ImGui::CollapsingHeader(ICON_MD_FAVORITE " Health", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (character) {
            int currentHealth = character->GetHealth();
            int maxHealth = character->GetMaxHealth();
            int healthPercent = character->GetHealthInPercentage();

            ImGui::Text("Current: %d / %d (%d%%)", currentHealth, maxHealth, healthPercent);

            if (ImGui::SliderInt("Health (%)##character", &healthPercent, 0, 100)) {
                character->SetHealthInPercentage(healthPercent);
            }
            ImGui::SameLine(); 
            UI::HelpMarker("Sets health as a percentage of max health.");
        }

        if (HealthPatches::IsInitialized()) {
            bool freezeHealth = HealthPatches::IsFreezeHealthEnabled();
            if (ImGui::Checkbox("Infinite Health", &freezeHealth)) {
                HealthPatches::ToggleFreezeHealth();
            }
            ImGui::SameLine(); 
            UI::HelpMarker("Prevents your health from decreasing when taking damage.\n"
                "Credit: aSwedishMagyar & sanitka");
        }

        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##player"))
        {
            ImGui::Text("CPlayer Address: 0x%p", CNetworkPlayerManager::GetLocalPlayer());
            ImGui::Text("CCharacter Address from CPlayer: 0x%p", CNetworkPlayerManager::GetLocalPlayer()->GetCharacter());
            ImGui::Text("CCharacter Address from CNetworkPlayerManager: 0x%p", CNetworkPlayerManager::GetLocalPlayerCharacter());
            ImGui::Text("CNetworkPlayerComponent Address: 0x%p", CNetworkPlayerManager::GetLocalPlayerNetworkComponent());
            ImGui::Text("Player Count: %d", CNetworkPlayerManager::instance().GetPlayerCount());
            ImGui::TreePop();
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // -- STAMINA --
    auto* networkComp = CNetworkPlayerManager::GetLocalPlayerNetworkComponent();
    if (ImGui::CollapsingHeader(ICON_MD_FLASH_ON " Stamina", ImGuiTreeNodeFlags_DefaultOpen) && networkComp) {
        bool infiniteStamina = networkComp->GetInfiniteStamina();
        if (ImGui::Checkbox("Infinite Stamina", &infiniteStamina)) {
            networkComp->SetInfiniteStamina(infiniteStamina);
        }
        ImGui::SameLine(); 
        UI::HelpMarker("Prevents stamina from decreasing when performing actions like sprinting or jumping.",
            "Note: While this is active you cannot open the Building Menu (B)");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // -- INVENTORY --
    if (ImGui::CollapsingHeader(ICON_MD_LIST " Inventory", ImGuiTreeNodeFlags_DefaultOpen)) {
        // -- ammo
        if (AmmoDeployableConsumption::IsAmmoHookInitialized()) {
            bool infiniteAmmo = AmmoDeployableConsumption::IsAmmoConsumptionDisabled();
            if (ImGui::Checkbox("Infinite Ammo", &infiniteAmmo)) {
                AmmoDeployableConsumption::ToggleAmmoConsumption();
            }
            ImGui::SameLine();
            UI::HelpMarker("Prevents ammo from being consumed when firing weapons.");
        }
        // -- deployables / consumables
        if (AmmoDeployableConsumption::IsDeployableHookInitialized()) {
            bool infiniteDeployables = AmmoDeployableConsumption::IsDeployableConsumptionDisabled();
            if (ImGui::Checkbox("Infinite Deployables & Consumables", &infiniteDeployables)) {
                AmmoDeployableConsumption::ToggleDeployableConsumption();
            }
            ImGui::SameLine();
            UI::HelpMarker("Prevents deployable / consumable items (like turrets, mines, medkits etc.) from being used up.");
        }
        // -- resources
        if (ResourcePatches::IsInitialized()) {
            bool unlimitedResources = ResourcePatches::IsUnlimitedResourcesEnabled();
            if (ImGui::Checkbox("Unlimited Resources", &unlimitedResources)) {
                ResourcePatches::ToggleUnlimitedResources();
            }
            ImGui::SameLine();
            UI::HelpMarker("Prevents resources from being consumed when building structures or crafting items.",
                "Notice: You still need to have at least the required resources in your inventory to build or craft, they just won't be consumed."
                " (May not work in some cases)");
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // -- HUD --
    if (ImGui::CollapsingHeader(ICON_MD_DASHBOARD " HUD", ImGuiTreeNodeFlags_DefaultOpen) && UIPatches::IsInitialized()) {
        bool hideHUD = UIPatches::IsHideHUDEnabled();
        if (ImGui::Checkbox("Hide HUD", &hideHUD)) {
            UIPatches::ToggleHideHUD();
        }
        ImGui::SameLine();
        UI::HelpMarker("Toggles the in-game HUD visibility. When enabled, all HUD elements will be hidden from view. Hotkey can be configured in the Settings tab."
            "\nCredit: pigeon",
            "Warning: This breaks all other UIs such as the inventory, map, weapon wheel, etc. You'll need to disable this option again to use any UI.");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // -- VISIBILITY --
    if (ImGui::CollapsingHeader(ICON_MD_VISIBILITY " Visibility", ImGuiTreeNodeFlags_DefaultOpen) && DetectionPatch::IsInitialized())
    {
        bool undetectable = DetectionPatch::IsDetectionDisabled();
        if (ImGui::Checkbox("Undetectable by Machines", &undetectable)) {
            DetectionPatch::ToggleDetection();
        }
        ImGui::SameLine();
        UI::HelpMarker("Makes your character undetectable by enemy machines.\n"
            "Credit: aSwedishMagyar & sanitka");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    auto* playerInfo = CPlayerInformation::instance();

    // -- BASE BUILDING --
    if (ImGui::CollapsingHeader(ICON_MD_HOUSE " Base Building", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (BuildingPatches::IsInitialized())
        {
            bool freeBuild = BuildingPatches::IsFreeBuildEnabled();
            if (ImGui::Checkbox("Unrestricted Building", &freeBuild)) {
                BuildingPatches::ToggleFreeBuild();
            }
            ImGui::SameLine();
            UI::HelpMarker("Allows building structures without collision checks and ignores building limits.",
                "Warning: Do not build too close to the command center (crossed out tiles) or on tiles blocked due to terrain as these buildings will be deleted on game reload.");
        }

        if (playerInfo)
        {
            int commandTokens = playerInfo->GetCommandTokens();
            if (ImGui::InputInt("Command Tokens", &commandTokens, 5, 10)) {
                if (commandTokens < 0) commandTokens = 0;
                playerInfo->SetCommandTokens(commandTokens);
            }
            ImGui::SameLine();
            UI::HelpMarker("Sets your total command tokens. Used for claiming Control Points to build bases.");
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // -- VEHICLES --
    if (ImGui::CollapsingHeader(ICON_MD_PEDAL_BIKE " Vehicles", ImGuiTreeNodeFlags_DefaultOpen) && VehiclePatches::IsInitialized())
    {
        bool infiniteFuel = VehiclePatches::IsInfiniteFuelEnabled();
        if (ImGui::Checkbox("Infinite Vehicle Fuel", &infiniteFuel)) {
            VehiclePatches::ToggleInfiniteFuel();
        }
        ImGui::SameLine();
        UI::HelpMarker("Prevents vehicle fuel from decreasing while driving.\n"
                "Credit: aSwedishMagyar & sanitka");

        auto* vehicleManager = CVehicleManager::instance();
        if (vehicleManager)
        {
            auto* vehicle = vehicleManager->GetPlayerVehicle();

            if (vehicle && vehicle->IsPlayerInVehicle())
            {
                int currentHealth = vehicle->GetHealth();
                int maxHealth = vehicle->GetMaxHealth();
                int healthPercent = vehicle->GetHealthInPercentage();

                ImGui::Text("Current: %d / %d (%d%%)", currentHealth, maxHealth, healthPercent);

                if (ImGui::SliderInt("Health (%)##vehicle", &healthPercent, 0, 100)) {
                    vehicle->SetHealthInPercentage(healthPercent);
                }
                ImGui::SameLine();
                UI::HelpMarker("Sets health as a percentage of max health.",
                    "Note: This will not make your bike drivable after it has been destroyed.");
            } else {
                ImGui::Text("Enter a vehicle for more options.");
            }

            if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##vehicleManager"))
            {
                ImGui::Text("CVehicleManager Address: 0x%p", vehicleManager);
                ImGui::Text("Player Vehicle Address: 0x%p", vehicle);
                ImGui::Text("Is Player In Vehicle: %s", vehicle ? (vehicle->IsPlayerInVehicle() ? "Yes" : "No") : "N/A");
                ImGui::TreePop();
            }
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // -- SKILLS & EXPERIENCE --
    if (ImGui::CollapsingHeader(ICON_MD_ACCOUNT_CIRCLE " Skills & Experience", ImGuiTreeNodeFlags_DefaultOpen) && playerInfo) {
        int level = playerInfo->GetLevelForActive();
        if (ImGui::InputInt("Player Level", &level, 1, 5)) {
            if (level < 0) level = 0;
            if (level > 65535) level = 65535;
            playerInfo->SetLevelForActive((uint16_t)level);
        }
        ImGui::SameLine();
        UI::HelpMarker("Sets the level of your current character.",
            "Note: Changing your level does not automatically adjust your experience points. "
            "If your experience points are lower than the required amount for your new level, you will not be able to "
            "level up naturally until your experience points are 'caught up'.");

        int experience = playerInfo->GetExperienceForActive();
        if (ImGui::InputInt("Experience Points", &experience, 1000, 2000)) {
            if (experience < 0) experience = 0;
            playerInfo->SetExperienceForActive(experience);
        }
        ImGui::SameLine();
        UI::HelpMarker("Sets the experience points of your current character.",
            "Note: Changing your experience points does not automatically adjust your level. "
            "Gaining any amount of experience naturally in the game will set your level according to your experience points.");

        int skillPoints = playerInfo->GetSkillPointsForActive();
        if (ImGui::InputInt("Skill Points", &skillPoints, 1, 5)) {
            if (skillPoints < 0) skillPoints = 0;
            playerInfo->SetSkillPointsForActive(skillPoints);
        }
        ImGui::SameLine();
        UI::HelpMarker("Sets the available skill points of your current character.");

        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##playerInfo"))
        {
            ImGui::Text("CPlayerInformation Address: 0x%p", playerInfo);
            ImGui::Text("Active Player Index: %d", playerInfo->GetActivePlayer());
            ImGui::Text("Active Player Data Address: 0x%p", playerInfo->GetActivePlayerData());
            ImGui::TreePop();
        }
    }
}
} // namespace gz::UITabs
