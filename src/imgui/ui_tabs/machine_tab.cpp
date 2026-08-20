#include "../ui.h"
#include "../fonts/IconsMaterialDesign.h"

#include <imgui.h>

#include "game/animal_character.h"
#include "game/animal_network.h"
#include "game/animal_spotting_manager.h"
#include "game/camera_collision_modifier.h"
#include "game/damageable.h"
#include "game/reserve_world.h"
#include "game/spawn_system.h"
#include "game/data/spawnables_data.h"

namespace gz::UITabs
{
    void RenderMachineTab(CNetworkPlayerManager* manager)
    {
        UI* ui = UI::Get();
        ConsoleSettings& settings = ui->GetSettings();

        const CReserveWorld* reserveWorld = CReserveWorld::instance();
        const CPlayer* player = manager->GetPlayer();
        const CCharacter* character = player->GetCharacter();

        // -- SPAWNING --
        if (ImGui::CollapsingHeader(ICON_MD_ADD_CIRCLE " Spawning", ImGuiTreeNodeFlags_DefaultOpen)
            && reserveWorld && reserveWorld->GetPopulationManager() && reserveWorld->GetSpawnSystem())
        {
            if (ImGui::Checkbox("Force Spawn Machines", &settings.machinesForceSpawn))
            {
                // save setting
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker("Forces the spawning of machines at the requested position. Otherwise machines are only safely spawned in valid positions.",
                "If this is disabled and you try to spawn a machine in an invalid position, nothing will happen.");

            static bool hackMachineOnSpawn = false;
            ImGui::Checkbox("Spawn machines as friendly", &hackMachineOnSpawn);
            ImGui::SameLine();
            UI::HelpMarker("Spawned machines are instantly hacked.");

            ImGui::Indent();
            if (ImGui::CollapsingHeader("Machines"))
            {
                ImGui::Indent();
                for (const auto & category : Data::Spawnables::all_machines)
                {
                    if (ImGui::CollapsingHeader(category.displayName))
                    {
                        if (category.description)
                        {
                            ImGui::TextWrapped("%s", category.description);
                        }
                        if (ImGui::BeginTable("##spawnables_grid", 2, ImGuiTableFlags_SizingStretchSame))
                        {
                            for (size_t i = 0; i < category.count; i++)
                            {
                                const auto& spawnable = category.data[i];

                                ImGui::TableNextColumn();

                                // button for spawn
                                char buttonLabel[256];
                                snprintf(buttonLabel, sizeof(buttonLabel), "%s##0x%08X", spawnable.name, spawnable.hash);
                                if (ImGui::Button(buttonLabel, ImVec2(-FLT_MIN, 0))) // -FLT_MIN makes it fill column width
                                {
                                    CVector3f aimPos = player->GetAimPosition();
                                    CAnimal* animal = reserveWorld->GetSpawnSystem()->SpawnAtPosition(reserveWorld->m_PopulationManager,
                                        spawnable.tag, aimPos, settings.machinesForceSpawn);

                                    if (hackMachineOnSpawn && animal)
                                    {
                                        QueueHack(animal);
                                    }
                                }

                                // tooltip with details
                                if (ImGui::IsItemHovered())
                                {
                                    ImGui::BeginTooltip();
                                    ImGui::Text("%s - %s", category.displayName, spawnable.name);
                                    ImGui::Text("Tag: %s", spawnable.tag);
                                    ImGui::Text("Hash: 0x%08X", spawnable.hash);
                                    ImGui::Spacing();

                                    ImGui::Text("Machines will be spawned at your aim position. Spawning may take some time as the game handles that internally.");
                                    ImGui::Spacing();
                                    UI::WarningText("Spawning functionality is experimental and may cause issues. Use at your own risk!");
                                    UI::WarningText("Spawning may take some time, the game will decide when to spawn your machines. Avoid spamming the spawn buttons.");
                                    ImGui::EndTooltip();
                                }
                            }
                            ImGui::EndTable();
                        }
                    }
                }
                ImGui::Unindent();
            }
            ImGui::Unindent();
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // -- HACKING --
        if (ImGui::CollapsingHeader(ICON_MD_COMPUTER " Hacking", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Hacking Always Succeeds", &settings.hackingAlwaysSucceeds))
            {
                // save setting
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker("Makes all hacking attempts automatically succeed.");

            if (ImGui::Checkbox("Unlimited Hacking Time", &settings.unlimitedHackingTime))
            {
                // save setting
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker("Makes machines stay hacked for a really long time without needing to re-hack them.",
                           "Fun Fact: This isn't really unlimited, more like 3 hours.");

            if (ImGui::Checkbox("Advanced Hacking", &settings.alwaysAdvancedHacking))
            {
                // save setting
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker("If this is active, hacked machines will always attack other enemies."
                " This replicates the behavior of the 'Hacker' specialization skill and allows for having another specialization active instead.");
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // -- SPOTTED MACHINE --

        CAnimalSpottingManager* spottingManager = reserveWorld ? reserveWorld->GetSpottingManager() : nullptr;
        CRemoteController* rc = character ? character->GetRemoteController() : nullptr;
        if (ImGui::CollapsingHeader(ICON_MD_SMART_TOY " Targeted Machine", ImGuiTreeNodeFlags_DefaultOpen) && spottingManager)
        {
            if (spottingManager->GetTargetAnimal())
            {
                CAnimal* targetAnimal = spottingManager->GetTargetAnimal();
                CCharacter* machineChar = targetAnimal->GetSpawnedCharacter();
                CSpawnedAnimalNetworkComponent* component = targetAnimal->GetNetworkComponent();
                if (machineChar && character && component)
                {
                    ImGui::TextWrapped("Options for the currently targeted machine:");

                    ImGui::Spacing();

                    if (component->IsOwnedByLocalPlayer())
                    {
                        // -- invulnerability (only possible when machine is hosted by player)
                        auto* machineDmg = machineChar->GetDamageable();
                        bool invulnerable = machineDmg->IsInvulnerable();
                        if (ImGui::Checkbox("Invulnerable##machine", &invulnerable))
                        {
                            machineDmg->SetInvulnerable(invulnerable);
                        }
                        ImGui::SameLine();
                        UI::HelpMarker("Makes the targeted machine invulnerable to all damage.");

                        ImGui::SameLine();
                    }
                    // -- kill machine
                    if (ImGui::Button("Kill##target"))
                    {
                        targetAnimal->GetHealth()->ApplyDamage(999999);
                    }

                    // -- set machine health
                    // int healthPercent = targetAnimal->GetHealthPercentByLethalDamage();
                    // if (ImGui::SliderInt("Health (%)##machine", &healthPercent, 0, 100))
                    // {
                    //     targetAnimal->SetHealthInPercentageByLethalDamage(healthPercent);
                    // }

                    ImGui::Spacing();

                    // -- faction
                    int currentIndex = 0;
                    if (machineChar->GetFaction() == 2) currentIndex = 1;
                    else if (machineChar->GetFaction() == 5) currentIndex = 2;

                    if (ImGui::Combo("Faction", &currentIndex, "Resistance\0FNIX\0Soviets\0"))
                    {
                        // factions: 0 = player / humans / default, 2 = fnix, 5 = soviets
                        constexpr int factionValues[] = {0, 2, 5};
                        machineChar->SetFaction(factionValues[currentIndex]);
                    }
                    ImGui::SameLine();
                    UI::HelpMarker("Changes the faction of the targeted machine. It will become hostile towards all other factions than the selected one.");

                    // -- control machine (only if not already controlling one and machine is hosted by player)
                    if (rc && !character->IsControllingEntity() && component->IsOwnedByLocalPlayer())
                    {
                        ImGui::Spacing();

                        bool inRange = character->IsWithinInteractionRangeOf(machineChar);
                        float distance = character->GetDistanceTo(machineChar);
                        float maxRange = machineChar->GetInteractionRadius() * 0.5f;


                        if (!inRange)
                            ImGui::BeginDisabled();

                        if (ImGui::Button("Control Targeted Machine"))
                        {

                            if (character->GetFaction() != machineChar->GetFaction())
                            {

                                SHackRequestData payload = MakeGuaranteedHackPayload();
                                g_requestHack(component, &payload);
                            }
                            rc->RequestControlOfCharacter(machineChar);
                        }

                        if (!inRange)
                            ImGui::EndDisabled();

                        ImGui::SameLine();
                        UI::HelpMarker("Take control of the targeted machine. You will play as the machine until "
                                       "you release control, your player character dies or the controlled machine dies.",
                                       {
                                           "Soviet machines may have limited functionality and be hard to control.",
                                           "You can only use one primary and one secondary attack even if the machine has more weapons on it.",
                                           "Some machines have weapons equipped but you still may not be able to use them.",
                                           "If you have used the Remote-Controlled Tick in this game session, you won't be able to attack while controlling any other machine. Restart your game to be able to fully control machines again."
                                       });

                        if (!inRange)
                        {
                            char buffer[128];
                            snprintf(buffer, sizeof(buffer),
                                     "Too far away: %.1fm / max. %.1fm", distance, maxRange);
                            UI::WarningText(buffer, true);
                        }
                    } else if (!component->IsOwnedByLocalPlayer())
                    {
                        UI::WarningText("This machine is hosted by another player. You can't take control of it or make it invulnerable.", true);
                    }
                }

            } else {
                ImGui::TextWrapped("Look at a machine to modify it here.");
                UI::StartDisabledText();
                ImGui::TextWrapped("If you are looking at a machine and still see this message, try...");
                ImGui::Bullet(); ImGui::SameLine();
                ImGui::TextWrapped("moving closer to the machine");
                ImGui::Bullet(); ImGui::SameLine();
                ImGui::TextWrapped("switching your current weapon over the equipment wheel or the game hotkeys");
                UI::EndDisabledText();
            }

            if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##spotting"))
            {
                ImGui::Text("CAnimalSpottingManager Address: 0x%p", spottingManager);
                ImGui::Text("Target Animal: 0x%p", spottingManager->GetTargetAnimal());
                ImGui::Text("Last Target Animal: 0x%p", spottingManager->GetLastTargetAnimal());

                if (CAnimal* animal = spottingManager->GetTargetAnimal())
                {
                    CCharacter* machineChar = animal->GetSpawnedCharacter();
                    ImGui::Text("Target Animal CCharacter: 0x%p", machineChar);
                    ImGui::Spacing();
                    ImGui::Text("Target Animal m_Group: 0x%p", animal->m_Group);
                    ImGui::Text("Target Animal m_AnimalPopulation: 0x%p", animal->m_AnimalPopulation);
                    ImGui::Spacing();
                    CAnimalType* type = animal->GetAnimalType();
                    ImGui::Text("Target Animal CAnimalType: 0x%p", type);
                    ImGui::Text("Target Animal Name: %s", type->GetName());
                    ImGui::Text("Target Animal Spawn Tag: %s", type->GetSpawnTag());
                    ImGui::Text("Target Animal Machine Class: %s", type->GetMachineClass());
                    ImGui::Text("Target Animal Designator: %s", type->GetDesignator());
                    ImGui::Spacing();
                    ImGui::Text("CAnimalHealth Address: 0x%p", animal->GetHealth());
                    ImGui::Text("Target Animal Min Kill Damage: %.2f", animal->GetHealth()->m_MinKillDamage);
                    ImGui::Text("Target Animal Health (Lethal Dmg): %.2f", animal->GetHealthPercentByLethalDamage());
                    ImGui::Text("Target Animal Health (Max Health): %d", animal->GetHealthPercentByMaxHealth());
                    ImGui::Spacing();
                    CSpawnedAnimalNetworkComponent* comp = animal->GetNetworkComponent();
                    ImGui::Text("CSpawnedAnimalNetworkComponent Address: 0x%p", comp);
                    ImGui::Text("CAnimal from CSpawnedAnimalNetworkComponent: 0x%p", comp->GetAnimal());
                    ImGui::Spacing();
                    CAnimalCharacterComponent* animalComp = machineChar->GetAnimalComponent();
                    ImGui::Text("CAnimalCharacterComponent Address: 0x%p", animalComp);
                    if (ImGui::TreeNode("CDamageableCharacterParts"))
                    {
                        const auto parts = animalComp->GetAllParts();
                        for (size_t i = 0; i < parts.size(); i++)
                        {
                            const auto part = parts[i];
                            ImGui::Text("Part %zu (Health: %.2f / %.2f)", i, part->GetHealth(), part->GetMaxHealth());
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // -- CONTROLLED MACHINE --
        if (rc && ImGui::CollapsingHeader(ICON_MD_SETTINGS_REMOTE " Controlled Machine", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // -- signal strength cheat
            if (ImGui::Checkbox("Unlimited Signal Strength", &settings.unlimitedRcSignalStrength))
            {
                g_unlimitedSignalStrength = settings.unlimitedRcSignalStrength;
                // save setting
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker("Prevents signal loss when controlling machines at any distance.");

            // -- take control on hack
            if (ImGui::Checkbox("Control Machine on Hack", &settings.takeControlOnHack))
            {
                g_takeControlOnHack = settings.takeControlOnHack;
                // save setting
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            UI::HelpMarker(
                "Seamlessly take control of a machine when you successfully hack it with the binoculars or hacking dart.",
                {
                    "You must be within interaction range of the machine (usually 75m) for this to work.",
                    "If taking control fails, the machine will still be hacked, but you have to try taking control again.",
                    "You need to have 'Advanced Hacking' enabled or the 'Hacker' specialization active for this to work.",
                    "If you want to release control, use the button below or let your controlled machine die.",
                    "Due to limited controls, soviet machines will only be hacked and never automatically remote controlled.",
                    "Don't use this on your companion (use manual control in the 'Targeted Machine' tab instead).",
                    "If you have used the Remote-Controlled Tick in this game session, you won't be able to attack while controlling any other machine. Restart your game to be able to fully control machines again.",
                    "You can only use one primary and one secondary attack even if the machine has more weapons on it.",
                    "Some machines have weapons equipped but you still may not be able to use them."
                });

            ImGui::Spacing();

            if (auto* controlledChar = rc->GetControlledCharacter())
            {
                ImGui::TextWrapped("Options for the machine you are currently controlling:");
                ImGui::Spacing();
                // -- release control
                if (ImGui::Button("Release Control of Machine"))
                {
                    rc->ReleaseControl();
                }
                ImGui::SameLine();
                UI::HelpMarker("You are currently controlling a machine. Click this button to release control "
                    "and return to your player character.");

                ImGui::Spacing();

                // -- invulnerability
                auto* machineDmg = controlledChar->GetDamageable();
                bool invulnerable = machineDmg->IsInvulnerable();
                if (ImGui::Checkbox("Invulnerable##machine2", &invulnerable))
                {
                    machineDmg->SetInvulnerable(invulnerable);
                }
                ImGui::SameLine();
                UI::HelpMarker("Makes the machine you are controlling invulnerable to all damage.");

                ImGui::SameLine();
                // -- kill machine
                if (ImGui::Button("Kill##controlled"))
                {
                    machineDmg->SetHealth(0);
                }

                ImGui::Spacing();

                // -- detection
                bool isUndetectable = !controlledChar->IsDetectable();
                if (ImGui::Checkbox("Undetectable by other Machines", &isUndetectable)) {
                    controlledChar->SetDetectable(!isUndetectable);
                }
                ImGui::SameLine();
                UI::HelpMarker("Toggles whether the player will be detected by enemy machines.");

                ImGui::Spacing();

                // -- camera distance
                float cameraDistance = CCameraCollisionModifier::GetThirdPersonCameraDistance();
                if (ImGui::SliderFloat("Third Person Camera Distance", &cameraDistance, 0.0f, 1.0f))
                {
                    CCameraCollisionModifier::SetThirdPersonCameraDistance(cameraDistance);
                }
                ImGui::SameLine();
                UI::HelpMarker("Adjust the third person camera distance while controlling a machine (can also be changed by scrolling the mouse wheel)."
                               "1.0 = far away, 0.0 = very close",
                               "Note: Zooming in (scrolling up) is snappy and fast, meanwhile zooming out (scrolling down) will be slow and gradual. "
                               "This is normal game behavior.");
            }
            else
            {
                ImGui::TextWrapped("You are not currently controlling any machine.");
                UI::StartDisabledText();
                ImGui::TextWrapped("Take control of a machine from the 'Targeted Machine' tab above.");
                UI::EndDisabledText();
            }

            if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##remotecontrol"))
            {
                ImGui::Text("CRemoteController Address: 0x%p", rc);
                if (character)
                {
                    ImGui::Text("Player CCharacter is controlling entity: %s", character->IsControllingEntity() ? "Yes" : "No");
                }
                ImGui::Text("Controlled Entity CCharacter: 0x%p", rc->GetControlledCharacter());
                ImGui::Text("Controlled Entity Ref: 0x%p", rc->m_controlledEntityRef);
                ImGui::Text("CCameraObject: 0x%p", rc->GetControlledCameraObj());
                ImGui::TreePop();
            }
        }
    }
} // namespace gz::UITabs
