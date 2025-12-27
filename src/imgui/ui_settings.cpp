#include "ui_settings.h"

namespace gz
{

    std::string ConsoleSettings::GetKeyName(int vkCode)
    {
        char keyName[128];
        UINT scanCode = MapVirtualKeyA(vkCode, MAPVK_VK_TO_VSC);

        // for extended keys, set the extended key flag
        LONG lParam = scanCode << 16;
        if (vkCode == VK_INSERT || vkCode == VK_DELETE ||
            vkCode == VK_HOME || vkCode == VK_END ||
            vkCode == VK_PRIOR || vkCode == VK_NEXT)
        {
            lParam |= (1 << 24);
        }

        if (GetKeyNameTextA(lParam, keyName, sizeof(keyName)) > 0) {
            return std::string(keyName);
        }

        return "Unknown";
    }

    bool ConsoleSettings::IsValidKey(int vkCode)
    {
        // exclude mouse buttons
        if (vkCode >= VK_LBUTTON && vkCode <= VK_XBUTTON2) {
            return false;
        }

        // exclude system keys
        if (vkCode == VK_ESCAPE || vkCode == VK_TAB ||
            vkCode == VK_RETURN || vkCode == VK_BACK) {
            return false;
            }

        // exclude modifier-only keys
        if (vkCode == VK_SHIFT || vkCode == VK_CONTROL ||
            vkCode == VK_MENU || vkCode == VK_LWIN || vkCode == VK_RWIN) {
            return false;
            }

        // accept function keys (F1-F24)
        if (vkCode >= VK_F1 && vkCode <= VK_F24) {
            return true;
        }

        // accept alphanumeric keys (0-9, A-Z)
        if ((vkCode >= '0' && vkCode <= '9') || (vkCode >= 'A' && vkCode <= 'Z')) {
            return true;
        }

        // accept navigation keys
        if (vkCode >= VK_PRIOR && vkCode <= VK_DOWN) {
            return true;
        }

        // accept Insert, Delete
        if (vkCode == VK_INSERT || vkCode == VK_DELETE) {
            return true;
        }

        // accept numpad keys
        if (vkCode >= VK_NUMPAD0 && vkCode <= VK_DIVIDE)
        {
            return true;
        }

        // accept other common keys
        switch (vkCode) {
            case VK_OEM_1:      // ;
            case VK_OEM_PLUS:   // +
            case VK_OEM_COMMA:  // ,
            case VK_OEM_MINUS:  // -
            case VK_OEM_PERIOD: // .
            case VK_OEM_2:      // /
            case VK_OEM_3:      // ~
            case VK_OEM_4:      // [
            case VK_OEM_5:      // \|
            case VK_OEM_6:      // ]
            case VK_OEM_7:      // '
                return true;
            default:
                break;
        }

        return false;
    }

} // namespace gz