#include "../ui.h"
#include "../fonts/IconsMaterialDesign.h"
#include "game/player_network_manager.h"
#include "patches/building_patches.h"
#include "patches/resource_patch.h"

#include <imgui.h>

#include "game/map.h"
#include "game/player_info.h"
#include "game/vehicle.h"
#include "game/vehicle_manager.h"
#include "game/weapon_consumption.h"
#include "patches/fasttravel_patches.h"
#include "patches/vehicle_patches.h"

#include "game/animal_spotting_manager.h"
#include "game/deep_water.h"
#include "game/player_eq_utils.h"
#include "game/ui_manager.h"
#include "game/camera/camera_director.h"

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
            ImGui::Text("CNetworkPlayerManager Address: 0x%p", playerMgr);
            ImGui::Text("Player Count: %d", playerMgr->GetPlayerCount());

            ImGui::Spacing();
            CVector3f aimPos = playerMgr->GetPlayer()->GetAimPosition();
            ImGui::Text("Aim Position: (%.2f, %.2f, %.2f)", aimPos.x, aimPos.y, aimPos.z);
            CVector3f worldPos = playerMgr->GetPlayer()->GetPositionVector();
            ImGui::Text("World Position: (%.2f, %.2f, %.2f)", worldPos.x, worldPos.y, worldPos.z);
            float raycastDistance = playerMgr->GetPlayer()->GetAimRaycastDistance();
            ImGui::Text("Aim Raycast Distance: %.2f", raycastDistance);

            ImGui::Spacing();
            ImGui::Text("All players");
            auto* localNetworkPlayer = playerMgr->GetNetworkPlayer();
            for (int i = 0; i < 4; i++)
            {
                if (CNetworkPlayer* remotePlayer = playerMgr->GetRemotePlayer(i))
                {
                    std::string profileName = remotePlayer->GetProfileName();
                    bool isLocal = (remotePlayer == localNetworkPlayer);
                    CPlayer* player = remotePlayer->GetPlayer();
                    CCharacter* plCharacter = remotePlayer->GetCharacter();
                    CNetworkPlayerComponent* netComp = remotePlayer->GetNetworkComponent();

                    ImGui::Text("Player %d: %s%s", i, profileName.c_str(), isLocal ? " (Local Player)" : "");
                    ImGui::Text("    CNetworkPlayer Address: 0x%p", remotePlayer);
                    ImGui::Text("    CPlayer Address: 0x%p", player);
                    ImGui::Text("    CCharacter Address: 0x%p", plCharacter);
                    ImGui::Text("    CNetworkPlayerComponent Address: 0x%p", netComp);
                }
                else
                {
                    ImGui::Text("Player %d: <empty slot>", i);
                }
            }

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
    if (ImGui::CollapsingHeader(ICON_MD_DASHBOARD " HUD", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool hideHUD = CUIManager::IsHudHidden();
        if (ImGui::Checkbox("Hide HUD", &hideHUD)) {
            CUIManager::SetHideHud(hideHUD);
            // save setting
            settings.hideHud = hideHUD;
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
        ImGui::SameLine();
        UI::HelpMarker("Toggles the in-game HUD visibility. When enabled, all HUD elements will be hidden. Hotkey can be configured in the Settings tab.");
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

    CPlayerInformation* playerInfo = CPlayerInformation::instance();
    CLoginSaveLoader* saveLoader = playerInfo ? playerInfo->GetSaveLoader() : nullptr;
    SWorldSaveSlot* activeWorldSave = saveLoader ? saveLoader->GetActiveWorldSaveSlot() : nullptr;
    SPlayerData* activePlayerData = saveLoader ? saveLoader->GetActivePlayerData() : nullptr;

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

        if (activeWorldSave)
        {
            int commandTokens = activeWorldSave->GetCommandTokens();
            if (ImGui::InputInt("Command Tokens", &commandTokens, 5, 10)) {
                if (commandTokens < 0) commandTokens = 0;
                activeWorldSave->SetCommandTokens(commandTokens);
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
    if (ImGui::CollapsingHeader(ICON_MD_ACCOUNT_CIRCLE " Skills & Experience", ImGuiTreeNodeFlags_DefaultOpen) && playerInfo && saveLoader && activePlayerData) {
        CPlayerProgressionManager* progressionManager = playerInfo->GetProgressionManager();

        int level = activePlayerData->GetLevel();
        const int32_t levelCap = progressionManager->GetLevelCap();
        if (ImGui::InputInt("Player Level", &level, 1, 5)) {
            if (level < 0) level = 0;
            if (level > levelCap) level = levelCap;
            activePlayerData->SetLevel((uint16_t)level);
            // set experience to middle of the current level's XP range
            activePlayerData->SetExperience(
                progressionManager->GetXPAtLevel(level) + (progressionManager->GetXPAtLevel(level + 1) - progressionManager->GetXPAtLevel(level)) * 0.1f // 10% into the current level
                );
        }
        ImGui::SameLine();
        UI::HelpMarker("Sets the level of your current character. The experience points will be automatically adjusted to fit the new level.");

        int32_t experience = activePlayerData->GetExperience();
        const int32_t xpAtCurrentLevel = progressionManager->GetXPAtLevel(level);
        const int32_t xpAtNextLevel = progressionManager->GetXPAtNextLevel(level);
        if (ImGui::SliderInt("Experience", &experience, xpAtCurrentLevel, xpAtNextLevel - 1)) {
            // clamp experience to the current level's XP range
            if (experience < xpAtCurrentLevel) experience = xpAtCurrentLevel;
            if (experience >= xpAtNextLevel) experience = xpAtNextLevel - 1;
            // set experience
            activePlayerData->SetExperience(experience);
        }
        ImGui::SameLine();
        UI::HelpMarker("Sets the experience points of your current character."
            "The slider is limited to changing your XP within your current level. To change your level, use the 'Player Level' option above.");

        int skillPoints = activePlayerData->GetSkillPoints();
        if (ImGui::InputInt("Skill Points", &skillPoints, 1, 5)) {
            if (skillPoints < 0) skillPoints = 0;
            activePlayerData->SetSkillPoints(skillPoints);
        }
        ImGui::SameLine();
        UI::HelpMarker("Sets the available skill points of your current character.");

        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##playerInfo"))
        {
            ImGui::Text("CPlayerInformation Address: 0x%p", playerInfo);
            ImGui::Text("CLoginSaveLoader Address: 0x%p", saveLoader);
            ImGui::Text("CPlayerProgressionManager Address: 0x%p", progressionManager);
            ImGui::Spacing();
            ImGui::Text("Active Player Index: %d", saveLoader->GetActivePlayerIndex());
            ImGui::Text("Active Player Data Address: 0x%p", activePlayerData);
            ImGui::Text("Active World Save Slot Index: %d", saveLoader->GetActiveWorldIndex());
            ImGui::Text("Active World Save Slot Address: 0x%p", activeWorldSave);
            ImGui::Text("Level Cap: %d", progressionManager->GetLevelCap());
            ImGui::Text("XP for Next Level: %d", progressionManager->GetXPAtNextLevel(activePlayerData->GetLevel()));
            ImGui::Text("Prestige Points: %d", progressionManager->GetPrestigePoints());
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

        ImGui::Spacing();

        if (character)
        {
            // -- deep water
            ImGui::Checkbox("Ignore Deep Water", &g_noDeepWaterEffects);
            ImGui::SameLine();
            UI::HelpMarker("Allows the player to travel underwater.");

            ImGui::Spacing();

            // -- ghost mode
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

    if (settings.showDebugInfo)
    {
        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader(ICON_MD_CAMERA " Camera [WIP]", ImGuiTreeNodeFlags_DefaultOpen))
        {
            UI::StartWarningText();
            ImGui::TextWrapped("Camera options are still in development, these options are incomplete.");
            UI::EndWarningText();

            if (auto* cameraDirector = CCameraDirector::instance())
            {
                if (ImGui::Button("Reset Camera"))
                {
                    g_forceEmoteCamera = false;
                    g_forceVehicleCamera = false;
                    g_forceRemoteCamera = false;
                    cameraDirector->ResetToDefaultCamera();
                }

                if (g_cameraIds.vehicleCached)
                {
                    if (ImGui::Button("Force Vehicle Camera"))
                    {
                        cameraDirector->PushCamera(&g_cameraIds.vehicle);
                        g_forceEmoteCamera = true;
                    }
                }
                else
                {
                    ImGui::TextDisabled("Vehicle camera not yet cached. Enter a vehicle once.");
                }
                if (g_cameraIds.emoteCached)
                {
                    if (ImGui::Button("Force Emote Camera"))
                    {
                        cameraDirector->PushCamera(&g_cameraIds.emote);
                        g_forceEmoteCamera = true;
                    }
                }
                else
                {
                    ImGui::TextDisabled("Emote camera not yet cached. Trigger a third person emote once.");
                }
                if (g_cameraIds.remoteCached)
                {
                    if (ImGui::Button("Force Remote Camera"))
                    {
                        cameraDirector->PushCamera(&g_cameraIds.remote);
                        g_forceRemoteCamera = true;
                    }
                }
                else
                {
                    ImGui::TextDisabled("Remote camera not yet cached. Take control of a machine once.");
                }

                if (character && ImGui::Button("Third Person Body (Blackboard)"))
                {
                    character->SetThirdPersonBodyVisible(true);
                }

                if (ImGui::TreeNode("Debug Info##cameradirector"))
                {
                    auto& sel = cameraDirector->m_SelectedCamera;
                    ImGui::Text("Selected Camera ID:");
                    ImGui::Text("  Hash: %04X %04X %04X  UserData: %04X",
                        sel.m_Hash[0], sel.m_Hash[1], sel.m_Hash[2], sel.m_UserData);

                    // default camera for reference
                    auto& def = cameraDirector->m_DefaultCamera;
                    ImGui::Text("Default Camera ID:");
                    ImGui::Text("  Hash: %04X %04X %04X  UserData: %04X",
                        def.m_Hash[0], def.m_Hash[1], def.m_Hash[2], def.m_UserData);


                    ImGui::Separator();
                    ImGui::Text("Selected (raw): %04X%04X%04X%04X",
                        sel.m_Hash[0], sel.m_Hash[1], sel.m_Hash[2], sel.m_UserData);

                    ImGui::TreePop();
                }
            }
        }
    }
}
} // namespace gz::UITabs
