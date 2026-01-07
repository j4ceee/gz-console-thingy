#include "ui_settings.h"

#include "game/event_scheduler.h"
#include "game/weapon_consumption.h"
#include "patches/building_patches.h"
#include "patches/fasttravel_patches.h"
#include "patches/resource_patch.h"
#include "patches/vehicle_patches.h"

namespace gz
{
    const std::vector<Setting>& ConsoleSettings::GetSettings() const
    {
        if (!m_cacheInitialized)
        {
            m_settingsCache.reserve(12); // pre-allocate space

            m_settingsCache.push_back({"ToggleUIKey",'i',const_cast<int*>(&toggleUIKey),
                nullptr
            });
            m_settingsCache.push_back({"TeleportToAimKey",'i',const_cast<int*>(&teleportToAimKey),
                nullptr
            });
            m_settingsCache.push_back({"HideHUDKey",'i',const_cast<int*>(&hideHUDKey),
                nullptr
            });
            m_settingsCache.push_back({"ShowDebugInfo",'b',const_cast<bool*>(&showDebugInfo),
                nullptr
            });
            m_settingsCache.push_back({"ShowCTHint",'b',const_cast<bool*>(&showHint),
                nullptr
            });
            m_settingsCache.push_back({"DisableAutoEvents",'b',const_cast<bool*>(&disableAutoEvents),
                [this]() {
                if (EventManager::Initialize()) {
                    EventManager::SetSchedulerBlocked(disableAutoEvents);
                }
            }});
            m_settingsCache.push_back({"EnableInfiniteAmmo",'b',const_cast<bool*>(&enableInfiniteAmmo),
                [this]() {
                if (AmmoDeployableConsumption::IsAmmoHookInitialized()) {
                    AmmoDeployableConsumption::EnableInfiniteAmmo(enableInfiniteAmmo);
                }
            }});
            m_settingsCache.push_back({"EnableInfiniteDeployables",'b',const_cast<bool*>(&enableInfiniteDeployables),
                [this]() {
                if (AmmoDeployableConsumption::IsDeployableHookInitialized()) {
                    AmmoDeployableConsumption::EnableInfiniteDeployables(enableInfiniteDeployables);
                }
            }});
            m_settingsCache.push_back({"EnableFastTravelAnywhere",'b',const_cast<bool*>(&enableFastTravelAnywhere),
                [this]() {
                if (FastTravelPatches::IsInitialized()) {
                    if (enableFastTravelAnywhere) {
                        FastTravelPatches::EnableFastTravelAnywhere();
                    } else {
                        FastTravelPatches::DisableFastTravelAnywhere();
                    }
                }
            }});
            m_settingsCache.push_back({"DisableBuildingRestrictions",'b',const_cast<bool*>(&enableUnrestrictedBuilding),
                [this]() {
                if (BuildingPatches::IsInitialized()) {
                    if (enableUnrestrictedBuilding) {
                        BuildingPatches::EnableFreeBuild();
                    } else {
                        BuildingPatches::DisableFreeBuild();
                    }
                }
            }});
            m_settingsCache.push_back({"EnableUnlimitedResources",'b',const_cast<bool*>(&enableUnlimitedResources),
                [this]() {
                if (ResourcePatches::IsInitialized()) {
                    if (enableUnlimitedResources) {
                        ResourcePatches::EnableUnlimitedResources();
                    } else {
                        ResourcePatches::DisableUnlimitedResources();
                    }
                }
            }});
            m_settingsCache.push_back({"EnableInfiniteVehicleFuel",'b',const_cast<bool*>(&enableInfiniteFuel),
                [this]() {
                if (VehiclePatches::IsInitialized()) {
                    if (enableInfiniteFuel) {
                        VehiclePatches::EnableInfiniteFuel();
                    } else {
                        VehiclePatches::DisableInfiniteFuel();
                    }
                }
            }});

            m_cacheInitialized = true;
        }

        return m_settingsCache;
    }

    void ConsoleSettings::ReadLine(const char* line)
    {
        const auto& settings = GetSettings();

        for (auto& setting : settings)
        {
            char format[64];
            snprintf(format, sizeof(format), "%s=%%d", setting.name);

            int value;
            if (sscanf_s(line, format, &value) == 1)
            {
                if (setting.type == 'i') {
                    *(int*)setting.ptr = value;
                }
                else if (setting.type == 'b') {
                    *(bool*)setting.ptr = (value != 0);
                }

                if (setting.onLoad) {
                    setting.onLoad();
                }

                return;
            }
        }
    }

    void ConsoleSettings::WriteAll(ImGuiTextBuffer* buf)
    {
        const auto& settings = GetSettings();

        for (const auto& setting : settings)
        {
            if (setting.type == 'i') {
                buf->appendf("%s=%d\n", setting.name, *(int*)setting.ptr);
            }
            else if (setting.type == 'b') {
                buf->appendf("%s=%d\n", setting.name, *(bool*)setting.ptr ? 1 : 0);
            }
        }
    }

    void ConsoleSettings::ApplyDefaults()
    {
        const auto& settings = GetSettings();

        for (auto& setting : settings)
        {
            if (setting.onLoad)
            {
                setting.onLoad();
            }
        }
    }

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