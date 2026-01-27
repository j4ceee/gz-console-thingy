#include "../ui.h"
#include "../fonts/IconsMaterialDesign.h"

#include <imgui.h>

#include "game/animal_spotting_manager.h"
#include "game/damageable.h"

namespace gz::UITabs
{
    void RenderMachineTab(CNetworkPlayerManager* playerMgr)
    {
        UI* ui = UI::Get();
        ConsoleSettings& settings = ui->GetSettings();

        auto* character = playerMgr->GetPlayer()->GetCharacter();

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
        CAnimalSpottingManager* spottingManager = CAnimalSpottingManager::instance();
        CRemoteController* rc = character ? character->GetRemoteController() : nullptr;
        if (ImGui::CollapsingHeader(ICON_MD_SMART_TOY " Targeted Machine", ImGuiTreeNodeFlags_DefaultOpen) && spottingManager)
        {
            if (spottingManager->GetTargetAnimal())
            {
                auto* machineChar = spottingManager->GetTargetAnimal()->GetSpawnedCharacter();
                if (machineChar)
                {
                    ImGui::TextWrapped("Options for the currently targeted machine:");

                    ImGui::Spacing();
                    // -- invulnerability
                    auto* machineDmg = machineChar->GetDamageable();
                    bool invulnerable = machineDmg->IsInvulnerable();
                    if (ImGui::Checkbox("Invulnerable##machine", &invulnerable))
                    {
                        machineDmg->SetInvulnerable(invulnerable);
                    }
                    ImGui::SameLine();
                    UI::HelpMarker("Makes the targeted machine invulnerable to all damage.");

                    ImGui::SameLine();
                    // -- kill machine
                    if (ImGui::Button("Kill"))
                    {
                        machineDmg->SetHealth(0);
                    }

                    ImGui::Spacing();

                    // -- faction
                    // factions: 0 = player / humans / default, 2 = fnix, 5 = soviets
                    const int factionValues[] = {0, 2, 5};
                    int currentIndex = 0;
                    if (machineChar->GetFaction() == 2) currentIndex = 1;
                    else if (machineChar->GetFaction() == 5) currentIndex = 2;

                    if (ImGui::Combo("Faction", &currentIndex, "Resistance\0FNIX\0Soviets\0"))
                    {
                        machineChar->SetFaction(factionValues[currentIndex]);
                    }
                    ImGui::SameLine();
                    UI::HelpMarker("Changes the faction of the targeted machine. It will become hostile towards all other factions than the selected one.");

                    // -- control machine (only if not already controlling one)
                    if (rc && !rc->GetControlledCharacter())
                    {
                        ImGui::Spacing();

                        bool inRange = character->IsWithinInteractionRange(machineChar);
                        float distance = character->GetDistanceTo(machineChar);
                        float maxRange = machineChar->GetInteractionRadius() * 0.5f;

                        if (!inRange)
                            ImGui::BeginDisabled();

                        if (ImGui::Button("Control Targeted Machine"))
                        {
                            int playerFaction = playerMgr->GetPlayer()->GetCharacter()->GetFaction();
                            machineChar->SetFaction(playerFaction);
                            rc->RequestControlOfCharacter(machineChar);
                        }

                        if (!inRange)
                            ImGui::EndDisabled();

                        ImGui::SameLine();
                        UI::HelpMarker("Take control of the targeted machine. You will play as the machine until "
                            "you release control, your player character dies or the controlled machine dies.",
                            "Warning: If you have used the Remote-Controlled Tick in this game session, you "
                            "won't be able to attack while controlling any other machine. Restart your game to be able to fully control machines again.");

                        if (!inRange)
                        {
                            char buffer[128];
                            snprintf(buffer, sizeof(buffer),
                                     "Too far away: %.1fm / max. %.1fm", distance, maxRange);
                            UI::WarningText(buffer, true);
                        }
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

                if (spottingManager->GetTargetAnimal())
                    ImGui::Text("Target Animal CCharacter: 0x%p",
                                spottingManager->GetTargetAnimal()->GetSpawnedCharacter());

                if (spottingManager->GetLastTargetAnimal())
                    ImGui::Text("Last Target Animal CCharacter: 0x%p",
                                spottingManager->GetLastTargetAnimal()->GetSpawnedCharacter());

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

            ImGui::Spacing();

            auto* controlledChar = rc->GetControlledCharacter();
            if (controlledChar)
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
                if (ImGui::Button("Kill"))
                {
                    machineDmg->SetHealth(0);
                }
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
                ImGui::Text("Controlled Entity CCharacter: 0x%p", rc->GetControlledCharacter());
                ImGui::TreePop();
            }
        }
    }
}
