#include "../ui.h"
#include "../fonts/IconsMaterialDesign.h"

#include <imgui.h>

#include "game/environment_gfx.h"
#include "game/event_scheduler.h"
#include "game/game_world.h"
#include "game/landscape_manager.h"
#include "game/physics_system.h"
#include "game/player_network_manager.h"
#include "game/spawn_system.h"
#include "game/weather.h"
#include "game/world_time.h"
#include "game/custom/preset_override.h"
#include "game/data/collectibles_data.h"
#include "game/data/data_funcs.h"
#include "game/data/spawnables_data.h"

namespace gz::UITabs
{
static char searchBuffer[256];
static std::string lastSearchTerm;

void RenderWorldTab(CNetworkPlayerManager* playerMgr)
{
    UI* ui = UI::Get();
    ConsoleSettings& settings = ui->GetSettings();

    // --- EVENTS ---
    CSocialManager* socialMgr = CSocialManager::instance();
    EventManager* eventMgr = socialMgr ? socialMgr->GetEventManager() : nullptr;
    if (ImGui::CollapsingHeader(ICON_MD_EVENT " Events", ImGuiTreeNodeFlags_DefaultOpen) && eventMgr) {
        auto events = eventMgr->GetActiveEvents();

        bool schedulerBlocked = EventManager::IsSchedulerBlocked();
        if (ImGui::Checkbox("Block automatic event scheduling", &schedulerBlocked)) {
            EventManager::SetSchedulerBlocked(schedulerBlocked);
            settings.disableAutoEvents = schedulerBlocked;
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
        ImGui::SameLine(); UI::HelpMarker("Prevents the game from changing active events automatically. Untick to let the game manage events as normal.",
            "Note: Events can only be changed once per game session. Disabling this option will not revert any active events. "
            "Restart your game and you will be able pick a new event.");

        if (events.empty()) {
            static int selectedEvent = 0;
            const char* eventNames[] = { "None", "Winter", "Lunar New Year", "Semla Event", "Halloween", "Anniversary" };

            if (ImGui::Combo("Select Event", &selectedEvent, eventNames, IM_ARRAYSIZE(eventNames))) {
                switch(selectedEvent) {
                    case 1: EventManager::SetCustomEvent(Events::WINTER); break;
                    case 2: EventManager::SetCustomEvent(Events::LUNAR_NEW_YEAR); break;
                    case 3: EventManager::SetCustomEvent(Events::SEMLA); break;
                    case 4: EventManager::SetCustomEvent(Events::HALLOWEEN); break;
                    case 5: EventManager::SetCustomEvent(Events::ANNIVERSARY); break;
                    default: break;
                }
            }
        } else {
            ImGui::Text("Active Event: %s", EventManager::GetEventName(events[0]));
            ImGui::SameLine();
            UI::HelpMarker("Currently active events in the game.",
            "Note: Events can only be changed once per game session. Restart your game if an event is already active and you want to change it.");
        }

        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##Events")) {
            ImGui::Text("Social Manager: %p", socialMgr);
            ImGui::Text("EventManager: %p", eventMgr);
            ImGui::Text("activeEventsStart: %p", eventMgr->activeEventsStart);
            ImGui::Text("activeEventsCurrent: %p", eventMgr->activeEventsCurrent);
            ImGui::Text("activeEventsEnd: %p", eventMgr->activeEventsEnd);
            ImGui::Text("Count: %td", eventMgr->activeEventsCurrent - eventMgr->activeEventsStart);

            ImGui::Text("Active Events: %zu", events.size());

            for (uint32_t hash : events) {
                ImGui::BulletText("%s (0x%08X)", EventManager::GetEventName(hash), hash);
            }
            ImGui::TreePop();
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // --- WORLD TIME ---
    auto* worldTime = CWorldTime::instance();
    if (ImGui::CollapsingHeader(ICON_MD_ACCESS_TIME " World Time", ImGuiTreeNodeFlags_DefaultOpen) && worldTime)
    {
        // --- MIRROR REAL TIME ---
        bool mirrorRealTime = settings.mirrorRealTime;
        if (ImGui::Checkbox("Mirror Real Time", &mirrorRealTime)) {
            settings.mirrorRealTime = mirrorRealTime;
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
        ImGui::SameLine();
        UI::HelpMarker("When enabled, the in-game time will automatically sync to your system clock, mirroring the current real-world time."
            "This allows you to experience the game's day / night cycle in real time.",
            "Note: When enabled, all manual time controls will be disabled. Disable this option to regain control over the in-game time.");

        if (mirrorRealTime)
            ImGui::BeginDisabled(); // disable manual time controls when mirroring real time

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --- TIME OF DAY SLIDER ---
        float time = worldTime->GetTime();
        if (ImGui::SliderFloat("Time of Day", &time, 0.0f, 24.0f, "")) {
            worldTime->SetTime(time);
        }

        // --- DIGITAL CLOCK (HH:MM) ---
        // Get current time decomposed
        int hours, minutes;
        worldTime->GetTimeAsHHMM(hours, minutes);
        bool timeChanged = false;

        ImGui::PushItemWidth(45.0f);
        if (ImGui::DragInt("##hours", &hours, 0.2f, 0, 23, "%02d")) {
            timeChanged = true;
        }
        ImGui::SameLine();
        ImGui::Text(":");
        ImGui::SameLine();
        if (ImGui::DragInt("##minutes", &minutes, 0.2f, 0, 59, "%02d")) {
            timeChanged = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        UI::HelpMarker("Set the in-game time of day. Time is represented in 24-hour format.\n"
                          "You can use the slider to set the time, or input hours and minutes directly.");

        if (timeChanged) {
            worldTime->SetTimeAsHHMM(hours, minutes);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --- PAUSE CONTROL ---
        bool isPaused = worldTime->IsPaused();
        if (ImGui::Checkbox("Pause Time", &isPaused)) {
            worldTime->SetPaused(isPaused);
        }
        ImGui::SameLine();
        UI::HelpMarker("Freezes the game world time completely when checked.\n"
                          "Uncheck to resume normal time progression.");

        // --- TIME SCALE ---
        float timeScale = worldTime->GetTimeScale();
        if (ImGui::SliderFloat("Time Scale", &timeScale, -1000.0f, 1000.0f, "%.2fx")) {
            worldTime->SetTimeScale(timeScale);
        }
        ImGui::SameLine();
        UI::HelpMarker("Adjust the speed at which time progresses in the game world.\n"
                          "A time scale of 1.0x represents normal speed, 0.0x freezes time, and negative values reverse time.\n"
                          "CTRL + Click the slider to input a custom value.");

        if (ImGui::Button(ICON_MD_REFRESH " Reset"))
        {
            worldTime->ResetTimeScale();
        }

        if (mirrorRealTime)
            ImGui::EndDisabled(); // disable manual time controls when mirroring real time
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // --- SPAWNING ---
    auto* spawnSys = CSpawnSystem::instance();
    if (ImGui::CollapsingHeader(ICON_MD_ADD_CIRCLE " Spawning", ImGuiTreeNodeFlags_DefaultOpen) && spawnSys)
    {
        ImGui::Indent();
        for (const auto & category : Data::spawnCategories)
        {
            if (ImGui::CollapsingHeader(category.displayName))
            {
                for (size_t idx = 0; idx < category.count; idx++)
                {
                    ImGui::Indent();

                    const auto & spawnableCategory = category.category[idx];

                    if (ImGui::CollapsingHeader(spawnableCategory.displayName))
                    {
                        if (spawnableCategory.description)
                        {
                            ImGui::TextWrapped("%s", spawnableCategory.description);
                        }
                        if (ImGui::BeginTable("##spawnables_grid", 2, ImGuiTableFlags_SizingStretchSame))
                        {
                            for (size_t i = 0; i < spawnableCategory.count; i++) {
                                const auto& spawnable = spawnableCategory.data[i];

                                ImGui::TableNextColumn();

                                // button for spawn
                                char buttonLabel[256];
                                snprintf(buttonLabel, sizeof(buttonLabel), "%s##0x%08X", spawnable.name, spawnable.hash);
                                if (ImGui::Button(buttonLabel, ImVec2(-FLT_MIN, 0))) { // -FLT_MIN makes it fill column width
                                    spawnSys->SpawnCategoryAtAimPosition(spawnable.tag, category.spawnSystemType);
                                }

                                // tooltip with details
                                if (ImGui::IsItemHovered()) {
                                    ImGui::BeginTooltip();
                                    ImGui::Text("%s", spawnable.name);
                                    ImGui::Text("Tag: %s", spawnable.tag);
                                    ImGui::Text("Hash: 0x%08X", spawnable.hash);
                                    ImGui::Spacing();

                                    ImGui::Text("Objects will be spawned at your aim position. Some objects will not despawn until you exit to menu.");

                                    ImGui::Spacing();

                                    UI::WarningText("Spawning functionality is experimental and may not work correctly for all objects.\n"
                                        "Use at your own risk!");

                                    if (category.spawnSystemType == 'v')
                                    {
                                        ImGui::Spacing();
                                        ImGui::Separator();
                                        ImGui::Text("For more vehicles please spawn a 'Vehicle Station' object from the 'Buildings & Props' category.");
                                    }
                                    ImGui::EndTooltip();
                                }
                            }
                            ImGui::EndTable();
                        }
                    }
                    ImGui::Unindent();
                }
            }
        }

        if (ImGui::CollapsingHeader("Custom"))
        {
            static char tagBuffer[128] = "skir_classa_load01";
            ImGui::InputText("Resource Tag", tagBuffer, sizeof(tagBuffer));

            static uint32_t typeId = 0x37C;
            static char typeIdHex[16] = "37C";

            ImGui::SetNextItemWidth(100.0f);
            if (ImGui::InputText("Type ID", typeIdHex, sizeof(typeIdHex), ImGuiInputTextFlags_CharsHexadecimal)) {
                typeId = (uint32_t)strtoul(typeIdHex, nullptr, 16);
            }
            ImGui::SameLine();
            ImGui::Text("(0x%X = %u)", typeId, typeId);

            if (ImGui::Button("Spawn at Aim Position")) {
                spawnSys->SpawnTagAtAimPosition(tagBuffer, typeId);
            }
        }

        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##Spawning")) {
            ImGui::Text("CSpawnSystem: %p", spawnSys);
            ImGui::TreePop();
        }

        ImGui::Unindent();
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // --- PHYSICS ---
    auto* physicsSystem = CPhysicsSystem::instance();
    auto* world = physicsSystem ? physicsSystem->GetWorld() : nullptr;
    if (ImGui::CollapsingHeader(ICON_MD_CATEGORY " Physics", ImGuiTreeNodeFlags_DefaultOpen) && world) {
        // -- world gravity
        float gravity = world->GetGravityInGs();
        ImGui::TextWrapped("World Gravity (Physics Objects)");
        if (ImGui::SliderFloat("Gravity (Gs)##world", &gravity, -10.0f, 10.0f, "%.2f Gs")) {
            world->SetGravityInGs(gravity);
        }
        ImGui::SameLine();
        UI::HelpMarker("Adjust the gravity of the game world in Gs (1 G = Earth's gravity).\n"
                          "Negative values will invert gravity (things fall upwards).",
                          "Note: This only affects physics-based objects, e.g. vehicles, ragdolled players, defeated enemies, etc.");

        if (ImGui::Button("Zero Gravity##world")) {
            world->SetGravityInGs(0.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Gravity##world")) {
            world->SetGravityInGs(1.0f);
        }

        if (CPfxCharacterInstance* pfxInstance = playerMgr->GetPlayer()->GetCharacter()->GetPfxInstance())
        {
            ImGui::Separator();

            float playerGravity = pfxInstance->GetGravityInGs();
            ImGui::TextWrapped("Player Gravity");
            if (ImGui::SliderFloat("Gravity (Gs)##player", &playerGravity, -25.0f, 25.0f, "%.2f Gs")) {
                pfxInstance->SetGravityInGs(playerGravity);
            }
            ImGui::SameLine();
            UI::HelpMarker("Adjust the gravity affecting the player character in Gs (1 G = Earth's gravity).\n"
                              "Negative values will invert gravity (you fall upwards).",
                              "Note: This only affects the player character. It does not affect any vehicles you are driving or other physics-based objects.");

            if (ImGui::Button("Zero Gravity##player")) {
                pfxInstance->SetGravityInGs(0.0f);
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset Gravity##player"))
            {
                pfxInstance->ResetGravityForPlayer();
            }
        }

        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##Physics")) {
            ImGui::Text("CPhysicsSystem: %p", physicsSystem);
            ImGui::Text("hknpWorld: %p", world);
            ImGui::Text("World Gravity: %.2f", world->GetGravity());

            if (CPfxCharacterInstance* pfxInstance = playerMgr->GetPlayer()->GetCharacter()->GetPfxInstance())
            {
                ImGui::Text("CPfxCharacterInstance Address: 0x%p", pfxInstance);
                ImGui::Text("Player Gravity: %.2f", pfxInstance->GetGravity());
                if (float groundDist = pfxInstance->GetGroundDistance(); groundDist >= FLT_MAX * 0.9f)
                    ImGui::Text("Distance to Ground: Too Far");
                else
                    ImGui::Text("Distance to Ground: %.2f", groundDist);
            }

            ImGui::TreePop();
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // --- TELEPORTATION ---
    auto* player = playerMgr->GetPlayer();
    auto* gameWorld = CGameWorld::instance();
    if (ImGui::CollapsingHeader(ICON_MD_PIN_DROP " Teleportation", ImGuiTreeNodeFlags_DefaultOpen) && player && gameWorld) {
        CVector3f    playerPos = player->GetPositionVector();
        static float x         = playerPos.x;
        static float y         = playerPos.y;
        static float z         = playerPos.z;

        if (ImGui::Button("Use Current Position")) {
            x = playerPos.x;
            y = playerPos.y;
            z = playerPos.z;
        }
        ImGui::InputFloat("X", &x, 1.0f, 10.0f, "%.1f");
        ImGui::InputFloat("Y", &y, 1.0f, 10.0f, "%.1f");
        ImGui::InputFloat("Z", &z, 1.0f, 10.0f, "%.1f");

        // 2 buttons side by side
        if (ImGui::Button("Teleport")) {
            gameWorld->TeleportToPositionXYZ(x, y, z);
        }
        UI::HoverTooltip("Teleport the player to the specified XYZ coordinates.");

        ImGui::SameLine();

        if (ImGui::Button("Teleport to Aim Position")) {
            gameWorld->TeleportToAimPosition(player);
        }
        UI::HoverTooltip(("Teleport the player to the position they are currently aiming at. "
            "This is generally the safest way to teleport. Hotkey: " + ConsoleSettings::GetKeyName(settings.teleportToAimKey)).c_str());

        ImGui::Spacing();

        ImGui::Indent();
        if (ImGui::CollapsingHeader(ICON_MD_LOCATION_ON " Predefined Locations"))
        {
            // search bar
            ImGui::SetNextItemWidth(-1); // Full width
            bool searchChanged = ImGui::InputTextWithHint("##search", "Search...", searchBuffer,
                                                          sizeof(searchBuffer));

            // cache filtered results when search changes
            if (searchChanged) {
                lastSearchTerm = searchBuffer;
            }

            ImGui::Spacing();

            // iterate through all categories
            for (const auto & category : Data::all_collectibles) {
                // count matches
                int matchCount = Data::CountMatches(category, searchBuffer);

                // hide categories with no matches
                if (matchCount == 0)
                    continue;

                // build header text with match count
                char headerText[256];
                if (searchBuffer[0] != '\0') {
                    snprintf(headerText, sizeof(headerText), "%s [%d/%zu]",
                             category.displayName, matchCount, category.count);
                } else {
                    snprintf(headerText, sizeof(headerText), "%s [%zu]",
                             category.displayName, category.count);
                }

                if (ImGui::CollapsingHeader(headerText)) {
                    if (ImGui::BeginTable("##collectibles_grid", 2, ImGuiTableFlags_SizingStretchSame)) {
                        for (size_t i = 0; i < category.count; i++) {
                            const auto& collectible = category.data[i];

                            // filter check
                            if (!Data::ContainsIgnoreCase(collectible.name, searchBuffer)) {
                                continue;
                            }

                            ImGui::TableNextColumn();

                            // button for teleport
                            char buttonLabel[256];
                            snprintf(buttonLabel, sizeof(buttonLabel), "%s##%s", collectible.name, collectible.collectable_id);
                            if (ImGui::Button(buttonLabel, ImVec2(-FLT_MIN, 0))) { // -FLT_MIN makes it fill column width
                                gameWorld->TeleportToPositionXYZ(collectible.x, collectible.y, collectible.z);
                            }

                            // tooltip with details
                            if (ImGui::IsItemHovered()) {
                                ImGui::BeginTooltip();
                                ImGui::Text("%s", collectible.name);
                                ImGui::Text("Position: %.1f, %.1f, %.1f", collectible.x, collectible.y, collectible.z);
                                if (collectible.description && collectible.description[0] != '\0') {
                                    ImGui::Separator();
                                    ImGui::PushTextWrapPos(400.0f);
                                    ImGui::Text("%s", Data::TruncateText(collectible.description, 500));
                                    ImGui::PopTextWrapPos();
                                }
                                ImGui::EndTooltip();
                            }
                        }
                        ImGui::EndTable();
                    }
                }
            }
        }
        ImGui::Unindent();
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // --- WEATHER ---
    const auto* landscapeManager = CLandscapeManager::instance();
    auto* weather = landscapeManager ? landscapeManager->GetWeather() : nullptr;
    auto* envGfxMgr = CEnvironmentGfxManager::instance();

    if (ImGui::CollapsingHeader(ICON_MD_WB_SUNNY " Weather", ImGuiTreeNodeFlags_DefaultOpen) && landscapeManager && weather)
    {
        {
            auto& wo = Weather::g_overrides;

            ImGui::Checkbox("Control Weather", &wo.enabled);
            ImGui::SameLine();
            UI::HelpMarker("Takes over the game's dynamic weather system.");

            ImGui::BeginDisabled(!wo.enabled);
            {
                ImGui::Indent();
                ImGui::Spacing();

                ImGui::SliderFloat("Cloud Density", &wo.cloudDensity, 0.0f, 8.0f, "%.2f");
                ImGui::SameLine();
                UI::HelpMarker("Control cloud thickness and with it rain and lightning. "
                               "Vanilla game range is ~1.6 to 6. Below ~2 the sky is clear, above ~5 it rains continuously.");

                ImGui::SliderFloat("Transition", &wo.transitionTime, 0.0f, 60.0f, "%.0f sec");
                ImGui::SameLine();
                UI::HelpMarker("How long the sky takes to reach the chosen cloud density.");

                ImGui::Spacing();
                ImGui::Checkbox("Manual Downpour", &wo.overrideDownpour);
                ImGui::SameLine();
                UI::HelpMarker("Decouples rain and snow from the clouds. Lets you have rain under a clear sky or an overcast sky with none.");

                ImGui::BeginDisabled(!wo.overrideDownpour);
                {
                    ImGui::SliderFloat("Downpour", &wo.downpour, 0.0f, 1.0f, "%.2f");
                    ImGui::SameLine();
                    UI::HelpMarker("Amount of rain / snow.");
                }
                ImGui::EndDisabled();

                ImGui::SliderFloat("Snow Ratio", &wo.snowRatio, 0.0f, 1.0f, "%.2f");
                ImGui::SameLine();
                UI::HelpMarker("Below 0.5 is rain, above 0.5 becomes snow.");

                ImGui::Spacing();

                ImGui::SliderFloat("Ground Snow", &wo.groundSnow, 0.0f, 1.0f, "%.2f");
                ImGui::SameLine();
                UI::HelpMarker("Amount of snow covering the world.");

                ImGui::SliderFloat("Snow Randomness", &wo.snowRandomness, 0.0f, 1.0f, "%.2f");
                ImGui::SameLine();
                UI::HelpMarker("How uniform or random the snow is. 0.0 is completely uniform, 1.0 is completely random.");

                ImGui::SliderFloat("Ground Wetness", &wo.groundWetness, 0.0f, 1.0f, "%.2f");
                ImGui::SameLine();
                UI::HelpMarker("How wet surfaces look.");

                ImGui::Spacing();

                ImGui::SliderFloat("Lightning", &wo.lightningChance, 0.0f, 20.0f, "%.2f");
                ImGui::SameLine();
                UI::HelpMarker("Chance of lightning in the clouds.",
                               "Needs thick clouds, has no effect below roughly 4 cloud density.");

                ImGui::SliderFloat("Cloud Height", &wo.cloudHeight, 0.0f, 3000.0f, "%.0f");
                ImGui::SameLine();
                UI::HelpMarker("The height at which the clouds appear.");

                ImGui::SliderFloat("Cloud Range", &wo.cloudThickness, 0.0f, 2000.0f, "%.0f");
                ImGui::SameLine();
                UI::HelpMarker("The distance from the cloud height upwards where clouds can form.");

                ImGui::Spacing();

                ImGui::SliderFloat("Wind Strength", &wo.gustStrength, 0.0f, 50.0f, "%.0f");
                ImGui::SameLine();
                UI::HelpMarker("The strength of the wind. Keep between 2 & 8 for best results.");

                ImGui::Spacing();
                if (ImGui::Button(ICON_MD_REFRESH " Reset to Defaults"))
                {
                    const bool wasEnabled = wo.enabled;
                    wo = Weather::Overrides{};
                    wo.enabled = wasEnabled;
                }

                ImGui::Unindent();
            }
            ImGui::EndDisabled();
        }

        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##Weather"))
        {
            auto* precipitation = weather->GetPrecipitation();
            auto* atmos = landscapeManager->GetAtmosphere();

            ImGui::Text("CLandscapeManager: 0x%p", landscapeManager);
            ImGui::Text("CAtmosphere: 0x%p", atmos);
            ImGui::Text("    render weather: %s", atmos->RenderWeather() ? "true" : "false");
            ImGui::Text("CWeather: 0x%p", weather);
            ImGui::Text("    cloud coverage: %.2f", weather->m_CloudCoverage);
            ImGui::Text("    next cloud coverage: %.2f", weather->m_NextCloudCoverage);
            ImGui::Text("    snow ration: %.2f", weather->m_SnowRatio);
            ImGui::Text("    snow amount: %.2f", weather->m_SnowAmount);
            ImGui::Text("    rain intensity: %.2f", weather->m_RainIntensity);
            ImGui::Text("    wetness: %.2f", weather->m_Wetness);
            ImGui::Text("    lightning intensity: %.2f", weather->m_LightningIntensity);
            ImGui::Text("    lightning possibility: %.2f", weather->m_LightningPossibility);
            ImGui::Text("    air density: %.2f", weather->m_AirDensity);
            ImGui::Text("    severity: %.2f -> %.2f (update in %.2f)", weather->m_Severity, weather->m_SeverityTarget, weather->m_SeverityUpdateTime);
            ImGui::Text("    severity min/max: %.2f / %.2f", weather->m_SeverityMin, weather->m_SeverityMax);
            ImGui::Text("    do update severity: %s", weather->m_DoUpdateSeverity ? "true" : "false");
            ImGui::Text("    transition time/freq: %.2f / %.2f", weather->m_SeverityTransitionTime, weather->m_SeverityTransitionFrequency);
            ImGui::Text("    precip threshold: %.2f", weather->m_PrecipitationThreshold);
            ImGui::Text("    visibility: %.2f", weather->m_Visibility);
            ImGui::Text("    force update: %d", weather->m_ForceUpdate);
            ImGui::Text("    cloud base/height: %.1f / %.1f", weather->m_CloudBase, weather->m_CloudHeight);
            ImGui::Text("CPrecipitation: 0x%p", precipitation);
            ImGui::Text("    intensity: %.2f", precipitation->m_Intensity);
            ImGui::Text("    amount: %.2f", precipitation->m_ConstantAmount);
            ImGui::Text("    snow ratio: %.2f", precipitation->m_SnowRatio);
            ImGui::Text("    user controlled: %s", precipitation->m_UserControlled ? "true" : "false");
            ImGui::Text("    enabled: %s", precipitation->m_Enabled ? "true" : "false");

            if (envGfxMgr)
            {
                ImGui::Text("CEnvironmentGfxManager: 0x%p", envGfxMgr);
                ImGui::Text("    m_weatherPresetsArray: 0x%p", envGfxMgr->m_weatherPresetsArray);
                ImGui::Text("    m_weatherPresetsEnd: 0x%p", envGfxMgr->m_weatherPresetsEnd);
                ImGui::Text("    m_weatherPresetHashes: 0x%p", envGfxMgr->m_weatherPresetHashes);
                ImGui::Text("    Preset Count: %zu", envGfxMgr->GetPresetCount());
                ImGui::Separator();
                ImGui::Text("Param Set Stack (%zu):", envGfxMgr->GetParamSetCount());
                for (auto* p = envGfxMgr->m_paramSetsBegin; p != envGfxMgr->m_paramSetsEnd; ++p)
                {
                    const auto* data = CEnvironmentGfxManager::GetWeatherPresetDataByHash(
                        p->m_ID < envGfxMgr->GetPresetCount() ? envGfxMgr->m_weatherPresetHashes[p->m_ID] : 0);

                    ImGui::BulletText("id 0x%08X prio %-4u blend %.2f  t %.1f/%.1f  res %p %s%s%s",
                                      p->m_ID, p->m_Priority, p->m_BlendValue, p->m_TimeBlend, p->m_TotalBlendTime,
                                      p->m_Parameters, (p->m_Flags & 1) ? "[in]" : "", (p->m_Flags & 2) ? "[out]" : "",
                                      data ? data->name : "");
                }
            }
            ImGui::TreePop();
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // --- GFX ---
    if (ImGui::CollapsingHeader(ICON_MD_FILTER_VINTAGE " GFX & Filters", ImGuiTreeNodeFlags_DefaultOpen) && envGfxMgr)
    {
        const uint32_t activeFilter = PresetOverride::ActiveFilterHash();

        ImGui::Indent();
        for (const auto& category : Data::Weather::all_gfx_presets)
        {
            if (ImGui::CollapsingHeader(category.displayName))
            {
                if (ImGui::BeginTable("##gfx_presets_grid", 3, ImGuiTableFlags_SizingStretchSame))
                {
                    for (size_t i = 0; i < category.count; i++)
                    {
                        const auto& preset = category.data[i];
                        ImGui::TableNextColumn();

                        const bool isActive = (activeFilter != 0 && preset.hash == activeFilter);
                        if (isActive)
                            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

                        char buttonLabel[256];
                        snprintf(buttonLabel, sizeof(buttonLabel), "%s##f0x%08X", preset.name, preset.hash);
                        if (ImGui::Button(buttonLabel, ImVec2(-FLT_MIN, 0)))
                        {
                            if (isActive)
                                PresetOverride::ClearFilter();
                            else
                                PresetOverride::ApplyFilter(preset.hash, 0.0f); // filters snap
                        }

                        if (isActive)
                            ImGui::PopStyleColor();
                    }
                    ImGui::EndTable();
                }
            }
        }
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::BeginDisabled(!PresetOverride::IsFilterActive());
        if (ImGui::Button(ICON_MD_REFRESH " Clear Filter"))
            PresetOverride::ClearFilter();
        ImGui::EndDisabled();
    }
}
} // namespace gz::UITabs