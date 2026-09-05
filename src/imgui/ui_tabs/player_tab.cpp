#include <imgui.h>

#include "../ui.h"
#include "../fonts/IconsMaterialDesign.h"
#include "game/player_network_manager.h"
#include "patches/resource_patch.h"
#include "game/map.h"
#include "game/player_info.h"
#include "game/vehicle.h"
#include "game/vehicle_manager.h"
#include "game/custom/weapon_consumption.h"
#include "patches/vehicle_patches.h"
#include "game/animal_spotting_manager.h"
#include "game/building_item.h"
#include "game/deep_water.h"
#include "game/network_manager.h"
#include "game/custom/player_eq_utils.h"
#include "game/player_spawn_manager.h"
#include "game/ui_manager.h"
#include "game/camera_director.h"
#include "game/camera_registry.h"
#include "game/custom/damage_multipliers.h"
#include "game/custom/third_person_camera.h"
#include "game/custom/vehicle_speed.h"

namespace gz::UITabs
{
    void RenderPlayerTab(CNetworkPlayerManager* playerMgr)
    {
        UI* ui = UI::Get();
        ConsoleSettings& settings = ui->GetSettings();

        CPlayer* player = playerMgr->GetPlayer();
        CCharacter* character = player->GetCharacter();
        CVehicleManager* vehicleManager = CVehicleManager::instance();

        // -- HEALTH --
        if (ImGui::CollapsingHeader(ICON_MD_FAVORITE " Health", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (character)
            {
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
                CVector3f aimPos = player->GetAimPosition();
                ImGui::Text("Aim Position: (%.2f, %.2f, %.2f)", aimPos.x, aimPos.y, aimPos.z);
                CVector3f worldPos = player->GetPositionVector();
                ImGui::Text("World Position: (%.2f, %.2f, %.2f)", worldPos.x, worldPos.y, worldPos.z);
                float raycastDistance = player->GetAimRaycastDistance();
                ImGui::Text("Aim Raycast Distance: %.2f", raycastDistance);

                auto* netMgr = CBaseNetworkManager::instance();
                ImGui::Spacing();
                ImGui::Text("CBaseNetworkManager Address: 0x%p", netMgr);
                ImGui::Text("Local Peer ID: %u", netMgr->GetLocalPeerID());
                ImGui::Text("Host Peer ID: %u", netMgr->GetHostPeerID());
                ImGui::Text("Local GUID: 0x%016llx", netMgr->GetLocalGuid());

                ImGui::Spacing();
                ImGui::Text("All players");
                auto* localNetworkPlayer = playerMgr->GetNetworkPlayer();
                for (int i = 0; i < 4; i++)
                {
                    if (CNetworkPlayer* remotePlayer = playerMgr->GetRemotePlayer(i))
                    {
                        std::string profileName = remotePlayer->GetProfileName();
                        bool isLocal = (remotePlayer == localNetworkPlayer);
                        bool isHost = remotePlayer->IsHost();
                        CPlayer* remPlayer = remotePlayer->GetPlayer();
                        CNetworkPlayerComponent* netComp = remotePlayer->GetNetworkComponent();
                        CCharacter* plCharacter = remotePlayer->GetCharacter();
                        CPfxCharacterInstance* pfxChar = plCharacter ? plCharacter->GetPfxInstance() : nullptr;

                        ImGui::Text("Player %d: %s %s %s", i, profileName.c_str(), isLocal ? "(Local Player)" : "",
                            isHost ? "(Host)" : "");
                        ImGui::Text("    CNetworkPlayer Address: 0x%p", remotePlayer);
                        ImGui::Text("    CPlayer Address: 0x%p", remPlayer);
                        ImGui::Text("    CCharacter Address: 0x%p", plCharacter);
                        ImGui::Text("    CPfxCharacterInstance Address: 0x%p", pfxChar);
                        ImGui::Text("    CNetworkPlayerComponent Address: 0x%p", netComp);
                        ImGui::Text("    Peer ID: %u", remotePlayer->m_peerID);
                        ImGui::Text("    GUID: 0x%016llx", remotePlayer->m_guid);
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
        if (ImGui::CollapsingHeader(ICON_MD_FLASH_ON " Stamina", ImGuiTreeNodeFlags_DefaultOpen) && networkComp)
        {
            bool infiniteStamina = networkComp->GetInfiniteStamina();
            if (ImGui::Checkbox("Infinite Stamina", &infiniteStamina))
            {
                networkComp->SetInfiniteStamina(infiniteStamina);
            }
            ImGui::SameLine();
            UI::HelpMarker("Prevents stamina from decreasing when performing actions like sprinting or jumping.",
                           "Note: While this is active you cannot open the Building Menu (B)");
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // -- INVENTORY --
        if (ImGui::CollapsingHeader(ICON_MD_LIST " Inventory", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool unlimitedStorageSize = PlayerEqUtils::IsUnlimitedStorageSize();
            bool ignoreEncumbrance = PlayerEqUtils::IsIgnoringEncumbrance();

            // -- encumbrance
            if (unlimitedStorageSize) ImGui::BeginDisabled();
            // if unlimited storage size is enabled -> disable carry weight option since it would be redundant
            if (ImGui::Checkbox("Unlimited Carry Weight", &ignoreEncumbrance))
            {
                PlayerEqUtils::SetIgnoreEncumbrance(ignoreEncumbrance);
                // save setting
                settings.unlimitedCarryWeight = ignoreEncumbrance;
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker(
                "Removes the carry weight limit, allowing the player to carry unlimited items without movement penalties.");
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
            UI::HelpMarker(
                "Removes the storage size limit for all containers, allowing them to hold unlimited items. This also includes the player's carry weight.",
                "Note: Close and reopen any currently open inventory UIs for the change to take effect.");

            ImGui::Spacing();

            // -- ammo
            if (AmmoDeployableConsumption::IsAmmoHookInitialized())
            {
                bool infiniteAmmo = AmmoDeployableConsumption::IsAmmoConsumptionDisabled();
                if (ImGui::Checkbox("Infinite Ammo", &infiniteAmmo))
                {
                    AmmoDeployableConsumption::ToggleAmmoConsumption();
                    // save setting
                    settings.enableInfiniteAmmo = infiniteAmmo;
                    ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                }
                ImGui::SameLine();
                UI::HelpMarker("Prevents ammo from being consumed when firing weapons.");
            }
            // -- deployables / consumables
            if (AmmoDeployableConsumption::IsDeployableHookInitialized())
            {
                bool infiniteDeployables = AmmoDeployableConsumption::IsDeployableConsumptionDisabled();
                if (ImGui::Checkbox("Infinite Deployables & Consumables", &infiniteDeployables))
                {
                    AmmoDeployableConsumption::ToggleDeployableConsumption();
                    // save setting
                    settings.enableInfiniteDeployables = infiniteDeployables;
                    ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                }
                ImGui::SameLine();
                UI::HelpMarker(
                    "Prevents deployable / consumable items (like turrets, mines, medkits etc.) from being used up.");
            }
            // -- resources
            if (ResourcePatches::IsInitialized())
            {
                bool unlimitedResources = ResourcePatches::IsUnlimitedResourcesEnabled();
                if (ImGui::Checkbox("Unlimited Resources", &unlimitedResources))
                {
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
        if (ImGui::CollapsingHeader(ICON_MD_DASHBOARD " HUD", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool hideHUD = CUIManager::IsHudHidden();
            if (ImGui::Checkbox("Hide HUD", &hideHUD))
            {
                CUIManager::SetHideHud(hideHUD);
                // save setting
                settings.hideHud = hideHUD;
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker(("Toggles the in-game HUD visibility. When enabled, all HUD elements will be hidden. "
                "Hotkey: " + ConsoleSettings::GetKeyName(settings.toggleUIKey)).c_str());
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader(ICON_MD_CAMERA " Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (TpState::g_resourcesPrimed && character)
            {
                if (character->IsControllingEntity() || (vehicleManager && vehicleManager->GetPlayerVehicle() &&
                    vehicleManager->GetPlayerVehicle()->IsPlayerInVehicle()))
                {
                    UI::WarningText("You can't switch to third person while controlling a machine or vehicle.", true);
                }
                else
                {
                    bool tp = CCharacter::IsThirdPersonActive();
                    if (ImGui::Checkbox("Third Person", &tp))
                        character->SetThirdPerson(tp);
                    ImGui::SameLine();
                    UI::HelpMarker(("Toggles between First Person & Third Person view. "
                                       " Hotkey: " + ConsoleSettings::GetKeyName(settings.thirdPersonKey) + " / 2x Right Stick on controllers").c_str(),
                                   {
                                       "Some behaviours may not work entirely in TP",
                                       "Reload animations will play twice in TP",
                                       "After switching to TP and back to FP once, reload weapon animations in FP will be delayed"
                                   });
                }
            }

            if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##camera"))
            {
                if (auto* cameraDirector = CCameraDirector::instance())
                {
                    if (ImGui::Button("Reset Camera"))
                    {
                        cameraDirector->ResetToDefaultCamera();
                    }

                    if (ImGui::TreeNode("Registered Cameras##cameraregistry"))
                    {
                        if (auto* registry = CCameraRegistry::instance())
                        {
                            const auto entries = registry->Enumerate();
                            ImGui::Text("%d registered cameras", (int)entries.size());
                            for (const auto& e : entries)
                            {
                                ImGui::Text("0x%08X id=%04X%04X%04X:%04X prio=%d %s",
                                            e.m_NameHash,
                                            e.m_Id.m_Hash[2], e.m_Id.m_Hash[1], e.m_Id.m_Hash[0],
                                            e.m_Id.m_UserData,
                                            e.m_Pipeline->m_Prio,
                                            CameraNames::Lookup(e.m_NameHash));
                                ImGui::SameLine();
                                ImGui::PushID(e.m_Pipeline);
                                if (ImGui::SmallButton("Push"))
                                {
                                    SCameraId id = e.m_Id;
                                    cameraDirector->PushCamera(&id);
                                }
                                ImGui::PopID();
                            }
                        }
                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Debug Info Camera Director##cameradirector"))
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

                        ImGui::Text("Selected (raw): %04X%04X%04X%04X",
                                    sel.m_Hash[0], sel.m_Hash[1], sel.m_Hash[2], sel.m_UserData);

                        ImGui::TreePop();
                    }
                }

                ImGui::Separator();

                if (character && ImGui::Button("Enable TP Body (Blackboard)"))
                {
                    character->SetThirdPersonBodyVisible(true);
                }
                ImGui::SameLine();
                if (character && ImGui::Button("Disable TP Body (Blackboard)"))
                {
                    character->SetThirdPersonBodyVisible(false);
                }

                ImGui::Separator();

                if (character && ImGui::TreeNode("Debug Info Anim Layers##animlayers"))
                {
                    auto* model = character->GetAnimatedModel();
                    auto* ctrl = model->GetAnimationControl();

                    // CCharacter::s_LayerNames, hashed with HASHING_FUNC_BUFF
                    auto knownLabel = [](uint32_t hash) -> const char*
                    {
                        switch (hash)
                        {
                        case 0x9011E763: return " (MAINBODY)";
                        case 0xA6468F52: return " (UPPERBODY)";
                        case 0x457905E1: return " (GLOBAL_PARTIAL)";
                        case 0xB2BE2993: return " (SYNCED_ADDITIVE)";
                        case 0x85E439F0: return " (GLOBAL_ADDITIVE)";
                        case 0x5A9EF178: return " (VOCALS)";
                        case 0xDEADBEEF: return " (unnamed)";
                        default: return "";
                        }
                    };

                    auto layerRow = [&](int i, const SAnimationLayerInstance* slot)
                    {
                        if (!slot) return;
                        ImGui::Text(
                            "  [%d] hash=0x%08X%s idx=%d sm=0x%p animSet=0x%p afsmHash=0x%08X asHash=0x%08X",
                            i,
                            slot->m_LayerHash, knownLabel(slot->m_LayerHash),
                            slot->m_LayerIndex,
                            slot->m_StateMachine.px,
                            slot->m_AnimSetHandle,
                            slot->m_AfsmFileHash,
                            slot->m_AsFileHash);
                    };

                    const int defaultCount = model->GetDefaultLayerCount();
                    ImGui::Text("m_DefaultLayers count: %d", defaultCount);
                    for (int i = 0; i < defaultCount; i++)
                        layerRow(i, model->GetDefaultLayerSlot(i));

                    ImGui::Separator();

                    const int externalCount = model->GetExternalLayerCount();
                    ImGui::Text("m_ExternalLayers count: %d", externalCount);
                    for (int i = 0; i < externalCount; i++)
                        layerRow(i, model->GetExternalLayerSlot(i));

                    ImGui::Separator();

                    const int hashCount = model->GetCurrentLayerHashCount();
                    ImGui::Text("m_CurrentLayerHashes count: %d", hashCount);
                    for (int i = 0; i < hashCount; i++)
                    {
                        const auto* hashes = model->GetCurrentLayerInfo(i);
                        if (!hashes) continue;
                        ImGui::Text("  [%d] afsmHash=0x%08X asHash=0x%08X",
                                    i, hashes->m_AfsmFileHash, hashes->m_AsFileHash);
                    }

                    ImGui::Separator();

                    const int animSetCount = model->GetAnimationSetCount();
                    ImGui::Text("m_AnimationSets count: %d", animSetCount);
                    for (int i = 0; i < animSetCount; i++)
                        ImGui::Text("  [%d] handle=0x%p", i, model->GetAnimationSetHandle(i));

                    ImGui::Separator();

                    const int ruleCount = model->GetRuleSystemCount();
                    ImGui::Text("m_RuleSystems count: %d / body parts: %d",
                                ruleCount, ctrl ? ctrl->GetBodyPartCount() : -1);
                    for (int i = 0; i < ruleCount && i < 8; i++)
                    {
                        const auto* mem = model->GetStateTaskMemory(i);
                        ImGui::Text("  [%d] rs=0x%p  taskMem=0x%p (%u bytes)  inactiveLayer=%d inactiveBody=%d",
                                    i,
                                    model->GetRuleSystem(i),
                                    mem ? mem->m_Memory : nullptr,
                                    mem ? mem->m_Size : 0,
                                    model->m_InactiveLayers[i] ? 1 : 0,
                                    ctrl ? (ctrl->m_InactiveBodyParts[i] ? 1 : 0) : -1);
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // -- MAP --
        if (ImGui::CollapsingHeader(ICON_MD_MAP " Map", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Enable Fast Travel Anywhere", &settings.enableFastTravelAnywhere))
            {
                g_fastTravelAnywhereEnabled = settings.enableFastTravelAnywhere;
                // save setting
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker(
                "Allows fast traveling to any icon on the map. You can even place custom waypoints and fast travel to them.");

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

        // -- DAMAGE, FACTIONS & DETECTION --
        if (ImGui::CollapsingHeader(ICON_MD_FLAG " Machine Interaction", ImGuiTreeNodeFlags_DefaultOpen) && character)
        {
            // -- machine dmg multiplier
            ImGui::SliderFloat("Bullet Damage Multiplier", &DamageMultipliers::g_applyBulletDamageMultiplier, 0.1f, 100.0f, "%.1f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                settings.machineBulletDmgMultiplier = DamageMultipliers::g_applyBulletDamageMultiplier;
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            } ImGui::SameLine(); UI::HelpMarker("Multiplies the damage dealt to machines by bullets.");

            // -- detection
            bool isUndetectable = !character->IsDetectable();
            if (ImGui::Checkbox("Undetectable by Machines", &isUndetectable))
            {
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
            UI::HelpMarker(
                "Changes the player's faction. The machines of the other factions than the selected one will be hostile towards you.");

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
        if (ImGui::CollapsingHeader(ICON_MD_HOUSE " Base Building", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Unrestricted Building", &settings.enableUnrestrictedBuilding))
            {
                g_forceSpawnLocationValid = settings.enableUnrestrictedBuilding;
                // save setting
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker("Allows building structures without collision checks and ignores building limits.",
                           "Warning: Do not build too close to the command center (crossed out tiles) or on tiles blocked due to terrain as these buildings will be deleted on game reload.");

            if (activeWorldSave)
            {
                int commandTokens = activeWorldSave->GetCommandTokens();
                if (ImGui::InputInt("Command Tokens", &commandTokens, 5, 10))
                {
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
        if (ImGui::CollapsingHeader(ICON_MD_PEDAL_BIKE " Vehicles", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (VehiclePatches::IsInitialized())
            {
                bool infiniteFuel = VehiclePatches::IsInfiniteFuelEnabled();
                if (ImGui::Checkbox("Infinite Vehicle Fuel", &infiniteFuel))
                {
                    VehiclePatches::ToggleInfiniteFuel();
                }
                ImGui::SameLine();
                UI::HelpMarker("Prevents vehicle fuel from decreasing while driving.\n"
                    "Credit: aSwedishMagyar & sanitka");
            }

            ImGui::SliderFloat("Vehicle Acceleration & Speed", &VehicleSpeed::g_VehicleSpeedMultiplier, 0.1f, 1000.0f, "%.1f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
            ImGui::SameLine(); UI::HelpMarker("Multiplies the speed of all vehicles.",
                "Speed of motorbikes is still limited.\nAir resistance will still limit the top speed, use the button below to disable it.");

            ImGui::Checkbox("No Air Resistance", &VehicleSpeed::g_NoAirResistance);
            ImGui::SameLine(); UI::HelpMarker("Removes aerodynamic drag from vehicles. Greatly increases top speed.");

            ImGui::Spacing();
            ImGui::Spacing();

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
                }
                else
                {
                    ImGui::Text("Spawn a vehicle for more options.");
                }

                if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##vehicleManager"))
                {
                    ImGui::Text("CVehicleManager Address: 0x%p", vehicleManager);
                    ImGui::Text("Player Vehicle Address: 0x%p", vehicle);
                    ImGui::Text("Is Player In Vehicle: %s",
                                vehicle ? (vehicle->IsPlayerInVehicle() ? "Yes" : "No") : "N/A");
                    ImGui::TreePop();
                }
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // -- SKILLS & EXPERIENCE --
        if (ImGui::CollapsingHeader(ICON_MD_ACCOUNT_CIRCLE " Skills & Experience", ImGuiTreeNodeFlags_DefaultOpen) &&
            playerInfo && saveLoader && activePlayerData)
        {
            CPlayerProgressionManager* progressionManager = playerInfo->GetProgressionManager();

            int level = activePlayerData->GetLevel();
            const int32_t levelCap = progressionManager->GetLevelCap();
            if (ImGui::InputInt("Player Level", &level, 1, 5))
            {
                if (level < 0) level = 0;
                if (level > levelCap) level = levelCap;
                activePlayerData->SetLevel((uint16_t)level);
                // set experience to middle of the current level's XP range
                activePlayerData->SetExperience(progressionManager->GetXPAtLevel(level) + (progressionManager->GetXPAtLevel(level + 1) -
                    progressionManager->GetXPAtLevel(level)) * 0.1f // 10% into the current level
                );
            }
            ImGui::SameLine();
            UI::HelpMarker(
                "Sets the level of your current character. The experience points will be automatically adjusted to fit the new level.");

            int32_t experience = activePlayerData->GetExperience();
            const int32_t xpAtCurrentLevel = progressionManager->GetXPAtLevel(level);
            const int32_t xpAtNextLevel = progressionManager->GetXPAtNextLevel(level);
            if (ImGui::SliderInt("Experience", &experience, xpAtCurrentLevel, xpAtNextLevel - 1))
            {
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
            if (ImGui::InputInt("Skill Points", &skillPoints, 1, 5))
            {
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
                if (ImGui::Checkbox("First-Person Shadow", &fpShadow))
                {
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
                if (ImGui::Button("Enable Ghost Mode"))
                {
                    character->SetGhostMode(true);
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_MD_REFRESH "##ghostMode"))
                {
                    character->SetGhostMode(false);
                }
                ImGui::SameLine();
                UI::HelpMarker(
                    "Enables ghost mode for the player, allowing to float through the environment and objects without collision.",
                    "Note: To disable ghost mode, press the refresh button or teleport (via hotkey or world tab).");
            }
        }
    }
} // namespace gz::UITabs
