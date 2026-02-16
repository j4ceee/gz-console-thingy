#include "../ui.h"
#include "../fonts/IconsMaterialDesign.h"
#include "game/player_network_manager.h"
#include "patches/building_patches.h"
#include "patches/resource_patch.h"

#include <imgui.h>

#include "game/damageable.h"
#include "game/map.h"
#include "game/player_info.h"
#include "game/vehicle.h"
#include "game/vehicle_manager.h"
#include "game/weapon_consumption.h"
#include "patches/fasttravel_patches.h"
#include "patches/ui_patches.h"
#include "patches/vehicle_patches.h"

#include "game/animal_spotting_manager.h"
#include "game/player_eq_utils.h"

namespace gz::UITabs
{
void RenderPlayerTab(CNetworkPlayerManager* playerMgr)
{
    UI* ui = UI::Get();
    ConsoleSettings& settings = ui->GetSettings();

    auto* character = playerMgr->GetPlayer()->GetCharacter();

    // -- HEALTH --
    if (ImGui::CollapsingHeader(ICON_MD_FAVORITE " Health", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (character) {
            auto* characterDmg = character->GetDamageable();
            UI::GetHealthModification(
                characterDmg,
                "character",
                "Makes the player invulnerable to all damage.",
                "",
                "Revive Player",
                "Revives the player if they are dead.",
                [character]() { character->Revive(); }
                );
        }

        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##player"))
        {
            ImGui::Text("CPlayer Address: 0x%p", playerMgr->GetPlayer());
            ImGui::Text("CAvatar Address from CCharacter: 0x%p", playerMgr->GetPlayer()->GetCharacter()->m_avatar);
            ImGui::Text("CCharacter Address from CPlayer: 0x%p", playerMgr->GetPlayer()->GetCharacter());
            ImGui::Text("CCharacter Address from CNetworkPlayer: 0x%p", playerMgr->GetCharacter());
            ImGui::Text("CNetworkPlayerComponent Address: 0x%p", playerMgr->GetPlayerNetworkComponent());
            ImGui::Text("Player Count: %d", playerMgr->GetPlayerCount());

            ImGui::Spacing();
            CVector3f aimPos = playerMgr->GetPlayer()->GetAimPosition();
            ImGui::Text("Aim Position: (%.2f, %.2f, %.2f)", aimPos.x, aimPos.y, aimPos.z);
            CVector3f worldPos = playerMgr->GetPlayer()->GetPositionVector();
            ImGui::Text("World Position: (%.2f, %.2f, %.2f)", worldPos.x, worldPos.y, worldPos.z);
            float raycastDistance = playerMgr->GetPlayer()->GetAimRaycastDistance();
            ImGui::Text("Aim Raycast Distance: %.2f", raycastDistance);

            ImGui::TreePop();
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // -- STAMINA --
    auto* networkComp = playerMgr->GetPlayerNetworkComponent();
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

        bool unlimitedStorageSize = PlayerEqUtils::IsUnlimitedStorageSize();
        bool ignoreEncumbrance = PlayerEqUtils::IsIgnoringEncumbrance();

        // -- encumbrance
        if (unlimitedStorageSize) ImGui::BeginDisabled(); // if unlimited storage size is enabled -> disable carry weight option since it would be redundant
        if (ImGui::Checkbox("Unlimited Carry Weight", &ignoreEncumbrance))
        {
            PlayerEqUtils::SetIgnoreEncumbrance(ignoreEncumbrance);
            // save setting
            settings.unlimitedCarryWeight = ignoreEncumbrance;
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
        ImGui::SameLine();
        UI::HelpMarker("Removes the carry weight limit, allowing the player to carry unlimited items without movement penalties.");
        if (unlimitedStorageSize) ImGui::EndDisabled();

        // -- storage size
        if (ImGui::Checkbox("Unlimited Storage Size", &unlimitedStorageSize))
        {
            PlayerEqUtils::SetUnlimitedStorageSize(unlimitedStorageSize);
            // save setting
            settings.unlimitedStorageSize = unlimitedStorageSize;
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
        ImGui::SameLine();
        UI::HelpMarker("Removes the storage size limit for all containers, allowing them to hold unlimited items. This also includes the player's carry weight.",
            "Note: Close and reopen any currently open inventory UIs for the change to take effect.");

        ImGui::Spacing();

        // -- ammo
        if (AmmoDeployableConsumption::IsAmmoHookInitialized()) {
            bool infiniteAmmo = AmmoDeployableConsumption::IsAmmoConsumptionDisabled();
            if (ImGui::Checkbox("Infinite Ammo", &infiniteAmmo)) {
                AmmoDeployableConsumption::ToggleAmmoConsumption();
                // save setting
                settings.enableInfiniteAmmo = infiniteAmmo;
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker("Prevents ammo from being consumed when firing weapons.");
        }
        // -- deployables / consumables
        if (AmmoDeployableConsumption::IsDeployableHookInitialized()) {
            bool infiniteDeployables = AmmoDeployableConsumption::IsDeployableConsumptionDisabled();
            if (ImGui::Checkbox("Infinite Deployables & Consumables", &infiniteDeployables)) {
                AmmoDeployableConsumption::ToggleDeployableConsumption();
                // save setting
                settings.enableInfiniteDeployables = infiniteDeployables;
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
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

    // -- MAP --
    if (ImGui::CollapsingHeader(ICON_MD_MAP " Map", ImGuiTreeNodeFlags_DefaultOpen) && FastTravelPatches::IsInitialized()) {
        bool enableFastTravelAnywhere = FastTravelPatches::IsFastTravelAnywhereEnabled();
        if (ImGui::Checkbox("Enable Fast Travel Anywhere", &enableFastTravelAnywhere)) {
            FastTravelPatches::ToggleFastTravelAnywhere();
            // save setting
            settings.enableFastTravelAnywhere = enableFastTravelAnywhere;
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
        ImGui::SameLine();
        UI::HelpMarker("Allows fast traveling to any icon on the map. You can even place custom waypoints and fast travel to them.");

        CMap* map = CMap::instance();
        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##map") && map)
        {
            ImGui::Text("CMap Address: 0x%p", map);
            ImGui::Text("Map Center: (%.2f, %.2f, %.2f)", map->mapCenterX, map->mapCenterY, map->mapCenterZ);
            ImGui::Text("Cursor Screen Position: (%.2f, %.2f)", map->cursorScreenX, map->cursorScreenY);
            ImGui::Text("Map Scale: %.2f / Map Scale 2: %.2f", map->mapScale, map->mapScale2);
            ImGui::TreePop();
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // -- FACTIONS & DETECTION --
    if (ImGui::CollapsingHeader(ICON_MD_FLAG " Machine Interaction", ImGuiTreeNodeFlags_DefaultOpen) && character)
    {
        // -- detection
        bool isUndetectable = !character->IsDetectable();
        if (ImGui::Checkbox("Undetectable by Machines", &isUndetectable)) {
            character->SetDetectable(!isUndetectable);
        }
        ImGui::SameLine();
        UI::HelpMarker("Toggles whether the player will be detected by enemy machines.");

        // -- faction
        // factions: 0 = player / humans / default, 2 = fnix, 5 = soviets
        const int factionValues[] = {0, 2, 5};
        int currentIndex = 0;
        if (character->GetFaction() == 2) currentIndex = 1;
        else if (character->GetFaction() == 5) currentIndex = 2;

        if (ImGui::Combo("Faction", &currentIndex, "Resistance\0FNIX\0Soviets\0"))
        {
            character->SetFaction(factionValues[currentIndex]);
        }
        ImGui::SameLine();
        UI::HelpMarker("Changes the player's faction. The machines of the other factions than the selected one will be hostile towards you.");

        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##factions"))
        {
            ImGui::Text("CCharacter Address: 0x%p", character);
            ImGui::Text("Original Faction: %d", character->GetOriginalFaction());
            ImGui::Text("Current Faction: %d", character->GetFaction());
            ImGui::Text("Is Detectable: %s", character->IsDetectable() ? "Yes" : "No");
            ImGui::TreePop();
        }
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
                // save setting
                settings.enableUnrestrictedBuilding = freeBuild;
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
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

        ImGui::Spacing();
        ImGui::Spacing();

        auto* vehicleManager = CVehicleManager::instance();
        if (vehicleManager)
        {
            auto* vehicle = vehicleManager->GetPlayerVehicle();

            if (vehicle)
            {
                auto* vehicleDmg = vehicle->GetDamageable();

                UI::GetHealthModification(
                    vehicleDmg,
                    "vehicle",
                    "Makes the vehicle invulnerable to all damage.",
                    "Note: This will not make your bike drivable after it has been destroyed."
                );
            } else {
                ImGui::Text("Spawn a vehicle for more options.");
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

    ImGui::Spacing();
    ImGui::Spacing();

    // -- MISCELLANEOUS --
    if (ImGui::CollapsingHeader(ICON_MD_TUNE " Miscellaneous", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // -- first-person shadow
        auto player = playerMgr->GetPlayer();
        if (player)
        {
            bool fpShadow = CPlayer::GetFPPlayerShadowEnabled();
            if (ImGui::Checkbox("First-Person Shadow", &fpShadow)) {
                CPlayer::SetFPPlayerShadowEnabled(fpShadow);
                // save setting
                settings.fpPlayerShadow = fpShadow;
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker("Toggles the visibility of the player's shadow in first-person view.",
                "Note: When toggling in-game you'll need to edit your character's appearance (e.g., change clothes) for the change to take effect.");
        }

        // -- ghost mode
        if (character)
        {
            if (ImGui::Button("Enable Ghost Mode")) {
                character->SetGhostMode(true);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_MD_REFRESH "##ghostMode"))
            {
                character->SetGhostMode(false);
            }
            ImGui::SameLine();
            UI::HelpMarker("Enables ghost mode for the player, allowing to float through the environment and objects without collision.",
                "Note: To disable ghost mode, press the refresh button or teleport (via hotkey or world tab).");
        }

    }
}
} // namespace gz::UITabs
