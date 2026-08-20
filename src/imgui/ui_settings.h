#pragma once

#include <functional>
#include <string>
#include <Windows.h>
#include "imgui.h"

namespace gz
{
    // Setting descriptor
    struct Setting {
        const char* name;               // name in config file
        char type;                      // 'i' = int, 'b' = bool, 'f' = float
        void* ptr;                      // pointer to member variable
        std::function<void()> onLoad;   // optional callback after loading
    };

    struct ConsoleSettings
    {
        int toggleUIKey = VK_F1;
        int teleportToAimKey = VK_F2;
        int hideHUDKey = VK_F3;

        float fontSize = 1.0f;
        float lastAppliedFontSize = 1.0f;
        bool showDebugInfo = false;
        bool showHint = true;
        bool quickerStartup = false;

        bool hideHud = false;
        bool disableAutoEvents = true;
        bool enableInfiniteAmmo = false;
        bool enableInfiniteDeployables = false;
        bool enableFastTravelAnywhere = true;
        bool enableUnrestrictedBuilding = true;
        bool enableUnlimitedResources = false;
        bool enableInfiniteFuel = false;
        bool enableInfiniteBikeHealth = false;
        bool fpPlayerShadow = false;
        bool hackingAlwaysSucceeds = false;
        bool unlimitedHackingTime = false;
        bool alwaysAdvancedHacking = false;
        bool unlimitedRcSignalStrength = true;
        bool takeControlOnHack = false;
        bool unlimitedCarryWeight = false;
        bool unlimitedStorageSize = false;
        bool mirrorRealTime = false;
        bool machinesForceSpawn = true;

        mutable std::vector<Setting> m_settingsCache;
        mutable bool m_cacheInitialized = false;

        const std::vector<Setting>& GetSettings() const;

        // Serialization methods
        void ReadLine(const char* line) const;
        void WriteAll(ImGuiTextBuffer* buf) const;
        void ApplyDefaults() const;

        // helper methods
        static std::string GetKeyName(int vkCode);
        static bool IsValidKey(int vkCode);

        static void ApplyFontSize();
    };

} // namespace gz