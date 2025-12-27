#include "../ui.h"
#include "../fonts/IconsMaterialDesign.h"

#include <imgui.h>

#include "game/environment_gfx.h"
#include "game/event_scheduler.h"
#include "game/game_world.h"
#include "game/physics_system.h"
#include "game/player_manager.h"
#include "game/spawn_system.h"
#include "game/world_time.h"
#include "game/data/collectibles_data.h"
#include "game/data/data_funcs.h"
#include "game/data/spawnables_data.h"
#include "patches/cloud_patch.h"

namespace gz::UITabs
{
static char searchBuffer[256];
static std::string lastSearchTerm;

void RenderWorldTab()
{
    UI* ui = UI::Get();
    ConsoleSettings& settings = ui->GetSettings();

    // --- EVENTS ---
    EventManager* eventMgr = GetEventManager();
    if (ImGui::CollapsingHeader(ICON_MD_EVENT " Events", ImGuiTreeNodeFlags_DefaultOpen) && eventMgr) {
        auto events = eventMgr->GetActiveEvents();

        bool schedulerBlocked = IsSchedulerBlocked();
        if (ImGui::Checkbox("Block automatic event scheduling", &schedulerBlocked)) {
            SetSchedulerBlocked(schedulerBlocked);
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
                    case 1: SetCustomEvent(Events::WINTER); break;
                    case 2: SetCustomEvent(Events::LUNAR_NEW_YEAR); break;
                    case 3: SetCustomEvent(Events::SEMLA); break;
                    case 4: SetCustomEvent(Events::HALLOWEEN); break;
                    case 5: SetCustomEvent(Events::ANNIVERSARY); break;
                    default: break;
                }
            }
        } else {
            ImGui::Text("Active Event: %s", GetEventName(events[0]));
            ImGui::SameLine();
            UI::HelpMarker("Currently active events in the game.",
            "Note: Events can only be changed once per game session. Restart your game if an event is already active and you want to change it.");
        }

        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##Events")) {
            ImGui::Text("EventManager: %p", eventMgr);
            ImGui::Text("activeEventsStart: %p", eventMgr->activeEventsStart);
            ImGui::Text("activeEventsCurrent: %p", eventMgr->activeEventsCurrent);
            ImGui::Text("activeEventsEnd: %p", eventMgr->activeEventsEnd);
            ImGui::Text("Count: %td", eventMgr->activeEventsCurrent - eventMgr->activeEventsStart);

            ImGui::Text("Active Events: %zu", events.size());

            for (uint32_t hash : events) {
                ImGui::BulletText("%s (0x%08X)", GetEventName(hash), hash);
            }
            ImGui::TreePop();
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // --- TELEPORTATION ---
    auto* player = CNetworkPlayerManager::GetLocalPlayer();
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
        UI::HoverTooltip("Teleport the player to the specified XYZ coordinates.",
            "Warning: Teleporting too far from your current location may cause you to be stuck in the air.\n"
            "If this happens, you have to fast travel via the map to be able to move again.\n"
            "Unless you know that your destination is nearby it is generally recommended to primarily use the "
            "'Teleport to Aim Position' button or hotkey for safer teleportation.");

        ImGui::SameLine();

        if (ImGui::Button("Teleport to Aim Position")) {
            gameWorld->TeleportToAimPosition(player);
        }
        UI::HoverTooltip("Teleport the player to the position they are currently aiming at. "
            "This is generally the safest way to teleport. Can also be triggered via hotkey.");

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
                                    ImGui::TextWrapped("%s", Data::TruncateText(collectible.description, 500));
                                    ImGui::Separator();
                                    UI::WarningText("Warning: Teleporting too far from your current location may cause you to be stuck in the air.\n"
                                        "If this happens, you have to fast travel via the map to be able to move again.");
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

    // --- PHYSICS ---
    auto* physicsSystem = CPhysicsSystem::instance();
    auto* world = physicsSystem ? physicsSystem->GetWorld() : nullptr;
    if (ImGui::CollapsingHeader(ICON_MD_CATEGORY " Physics", ImGuiTreeNodeFlags_DefaultOpen) && world) {
        float gravity = world->GetGravityInGs();
        if (ImGui::SliderFloat("Gravity (Gs)", &gravity, -10.0f, 10.0f, "%.2f Gs")) {
            world->SetGravityInGs(gravity);
        }
        ImGui::SameLine();
        UI::HelpMarker("Adjust the gravity of the game world in Gs (1 G = Earth's gravity).\n"
                          "Negative values will invert gravity (things fall upwards).",
                          "Note: This only affects physics-based objects, e.g. vehicles, ragdolled players, defeated enemies, etc.");

        if (ImGui::Button("Zero Gravity")) {
            world->SetGravityInGs(0.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Gravity")) {
            world->SetGravityInGs(1.0f);
        }

        if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##Physics")) {
            ImGui::Text("CPhysicsSystem: %p", physicsSystem);
            ImGui::Text("hknpWorld: %p", world);
            ImGui::Text("Gravity: %.2f", world->GetGravity());
            ImGui::TreePop();
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // --- WORLD TIME ---
    auto* worldTime = CWorldTime::instance();
    if (ImGui::CollapsingHeader(ICON_MD_ACCESS_TIME " World Time", ImGuiTreeNodeFlags_DefaultOpen) && worldTime)
    {
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
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // --- WEATHER ---
    if (ImGui::CollapsingHeader(ICON_MD_WB_SUNNY " Weather & GFX", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (CloudPatch::IsInitialized())
        {
            bool hideClouds = CloudPatch::IsHideCloudsEnabled();
            if (ImGui::Checkbox("Hide Clouds", &hideClouds))
            {
                CloudPatch::ToggleHideClouds();
            }
            ImGui::SameLine();
            UI::HelpMarker("Toggles the visibility of clouds in the game world."
                "\nCredit: pigeon",
                "Note: Does not disable weather effects, only the clouds themselves.");

            ImGui::Separator();
        }

        auto* envGfxMgr = CEnvironmentGfxManager::instance();
        if (envGfxMgr)
        {
            auto* currentName = envGfxMgr->GetCurrentWeatherPresetName();
            ImGui::Text("Current Weather / GFX Preset: %s", currentName ? currentName : "Unknown");

            if (ImGui::Button("Restore Dynamic Weather / GFX")) {
                if (envGfxMgr->RestoreDynamicWeather()) {
                    Log("Dynamic weather restored.");
                } else {
                    Log("Failed to restore dynamic weather.");
                }
            }
            ImGui::SameLine();
            UI::HelpMarker("Restores the game's dynamic weather / GFX system, reverting any manually set weather / GFX presets.");

            ImGui::Separator();

            ImGui::Text("Available Weather / GFX Presets:");
            ImGui::Spacing();

            ImGui::Indent();
            for (const auto & category : Data::Weather::all_weather_presets) {
                if (ImGui::CollapsingHeader(category.displayName)) {
                    if (ImGui::BeginTable("##weather_presets_grid", 3, ImGuiTableFlags_SizingStretchSame)) {
                        for (size_t i = 0; i < category.count; i++) {
                            const auto& preset = category.data[i];

                            ImGui::TableNextColumn();

                            // button for setting weather preset
                            char buttonLabel[256];
                            snprintf(buttonLabel, sizeof(buttonLabel), "%s##0x%08X", preset.name, preset.hash);
                            if (ImGui::Button(buttonLabel, ImVec2(-FLT_MIN, 0))) { // -FLT_MIN makes it fill column width
                                if (envGfxMgr->m_isActive) {
                                    envGfxMgr->RestoreDynamicWeather();
                                }
                                if (envGfxMgr->SetWeatherPresetFromHash(preset.hash)) {
                                    Log("Weather preset set to: %s", preset.name);
                                } else {
                                    Log("Failed to set weather preset: %s", preset.name);
                                }
                            }
                            // tooltip with details
                            char tooltipText[256];
                            snprintf(tooltipText, sizeof(tooltipText), "Set the current weather / GFX preset to %s", preset.name);
                            UI::HoverTooltip(tooltipText,
                                "Note: Does not change clouds, rain, or other weather effects, only the visual GFX preset.");

                        }
                        ImGui::EndTable();
                    }
                }
            }
            ImGui::Unindent();

            if (settings.showDebugInfo && ImGui::TreeNode("Debug Info##Weather")) {
                ImGui::Text("CEnvironmentGfxManager: 0x%p", envGfxMgr);
                ImGui::Text("m_weatherPresetsArray: 0x%p", envGfxMgr->m_weatherPresetsArray);
                ImGui::Text("m_weatherPresetsEnd: 0x%p", envGfxMgr->m_weatherPresetsEnd);
                ImGui::Text("m_weatherPresetHashes: 0x%p", envGfxMgr->m_weatherPresetHashes);
                ImGui::Text("Preset Count: %zu", envGfxMgr->GetPresetCount());
                ImGui::Text("m_isActive: %s", envGfxMgr->m_isActive ? "true" : "false");
                ImGui::Text("m_currentPresetIndex: %d", envGfxMgr->m_currentPresetIndex);
                ImGui::Text("m_previousPresetIndex: %d", envGfxMgr->m_previousPresetIndex);
                ImGui::TreePop();
            }
        }
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
                                    else if (category.spawnSystemType == 'm')
                                    {
                                        ImGui::Spacing();
                                        ImGui::Separator();
                                        UI::WarningText("Spawning machines currently does not work properly.\n"
                                            "There are 2 workarounds available:\n"
                                            "- Ticks can be spawned via their 'eggs'\n"
                                            "- Lynx can be spawned via Firebird Vulture Beacon Drop");
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
        ImGui::Unindent();
    }
}
} // namespace gz::UITabs