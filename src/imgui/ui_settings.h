#pragma once

#include <string>
#include <Windows.h>

namespace gz
{

    struct ConsoleSettings
    {
        int toggleUIKey = VK_F1;
        int teleportToAimKey = VK_F2;
        int hideHUDKey = VK_F3;
        bool showDebugInfo = false;
        bool showHint = true;
        bool disableAutoEvents = true;

        // helper methods
        static std::string GetKeyName(int vkCode);
        static bool IsValidKey(int vkCode);
    };

} // namespace gz