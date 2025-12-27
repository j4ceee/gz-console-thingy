#include "FW1FontWrapper.h"

#include "addresses.h"

namespace gz
{
static uintptr_t g_Address[Address::COUNT] = {};

void InitAddresses(bool is_steam)
{
    HMODULE hModule = GetModuleHandle("GenerationZero_F.exe");
    uintptr_t base = (uintptr_t)hModule;
    uintptr_t offset = base - 0x140000000;

    g_Address[SANITY_CHECK] = (is_steam ? 0x141b60a40 : 0x141b60a40) + offset;
    g_Address[INST_CHARACTER_MANAGER] = (is_steam ? 0x142abed18 : 0x142abed18) + offset;
    g_Address[INST_GAME_WORLD] = (is_steam ? 0x142adcb60 : 0x142adcb60) + offset;
    g_Address[INST_INPUT_MANAGER] = (is_steam ? 0x142adff58 : 0x142adff58) + offset;
    g_Address[INST_NETWORK_PLAYER_MANAGER] = (is_steam ? 0x142adea38 : 0x142adea38) + offset;
    g_Address[INST_PLAYER_INFORMATION] = (is_steam ? 0x142abee30 : 0x142abee30) + offset;
    g_Address[INST_PHYSICS_SYSTEM] = (is_steam ? 0x142a35da0 : 0x142a35da0) + offset;
    g_Address[INST_ENVIRONMENT_GFX_MANAGER] = (is_steam ? 0x142a2b4e0 : 0x142a2b4e0) + offset;
    g_Address[INST_SPAWN_SYSTEM] = (is_steam ? 0x142abed20 : 0x142abed20) + offset;
    g_Address[INST_WORLDTIME] = (is_steam ? 0x142abdb48 : 0x142abdb48) + offset;
    g_Address[INST_MAP] = (is_steam ? 0x142adfec8 : 0x142adfec8) + offset;
    g_Address[INST_VEHICLE_MANAGER] = (is_steam ? 0x142abefe8 : 0x142abefe8) + offset;
    g_Address[WND_PROC] = (is_steam ? 0x1400a4b40 : 0x1400a4b40) + offset;
    g_Address[GRAPHICS_FLIP] = (is_steam ? 0x141784320 : 0x141784320) + offset;
    g_Address[VAR_GAME_STATE] = (is_steam ? 0x1428444e0 : 0x1428444e0) + offset;
    g_Address[VAR_CLOUD_VISIBILITY] = (is_steam ? 0x141bada08 : 0x141bada08) + offset;
    g_Address[ALLOC] = (is_steam ? 0x14009ddc0 : 0x14009ddc0) + offset;
    g_Address[FREE] = (is_steam ? 0x14185ca84 : 0x14185ca84) + offset;
    g_Address[SPAWN_SYSTEM_SPAWN] = (is_steam ? 0x140b83d90 : 0x140b83d90) + offset;
    g_Address[SPAWN_SYSTEM_PARSE_TAGS] = (is_steam ? 0x140b67c70 : 0x140b67c70) + offset;
    g_Address[SPAWN_SYSTEM_GET_MATCHING_RES] = (is_steam ? 0x140b53ba0 : 0x140b53ba0) + offset;
    g_Address[SPAWN_VEHICLE_OWNERSHIP_CALLBACK] = (is_steam ? 0x140937b80 : 0x140937b80) + offset;
    g_Address[EVENT_SCHEDULER] = (is_steam ? 0x140a02a10 : 0x140a02a10) + offset;
    g_Address[TELEPORT] = (is_steam ? 0x14085d210 : 0x14085d210) + offset;
    g_Address[SET_WEATHER_PRESET_FROM_HASH] = (is_steam ? 0x140129820 : 0x140129820) + offset;
    g_Address[RESTORE_DYNAMIC_WEATHER] = (is_steam ? 0x140134a00 : 0x140134a00) + offset;
    g_Address[WEATHER_UPDATE] = (is_steam ? 0x14012ca80 : 0x14012ca80) + offset;
    g_Address[HASHING_FUNC] = (is_steam ? 0x140de6b00 : 0x140de6b00) + offset;
    g_Address[FUNC_VSNPRINTF] = (is_steam ? 0x140de69b0 : 0x140de69b0) + offset;
    g_Address[FUNC_SCRIPT_ERROR_REPORT] = (is_steam ? 0x140e7eba0 : 0x140e7eba0) + offset;
    g_Address[CONSUME_DEPLOYABLES] = (is_steam ? 0x140717580 : 0x140717580) + offset;
    g_Address[CONSUME_AMMO] = (is_steam ? 0x140770ae0 : 0x140770ae0) + offset;
    g_Address[PATCH_BUILDING_UI_CHECK_1] = (is_steam ? 0x140725640 : 0x140725640) + offset;
    g_Address[PATCH_BUILDING_UI_CHECK_2] = (is_steam ? 0x140725816 : 0x140725816) + offset;
    g_Address[PATCH_BUILDING_CHECK_COLLISION] = (is_steam ? 0x140bbb00c : 0x140bbb00c) + offset;
    g_Address[PATCH_BUILDING_CHECK_MASTER] = (is_steam ? 0x140716080 : 0x140716080) + offset;
    g_Address[PATCH_RESOURCE_CONSUMPTION] = (is_steam ? 0x1407460ce : 0x1407460ce) + offset;
    g_Address[PATCH_HEALTH_GOD] = (is_steam ? 0x140654710 : 0x140654710) + offset;
    g_Address[PATCH_INFINITE_FUEL] = (is_steam ? 0x140d488f4 : 0x140d488f4) + offset;
    g_Address[PATCH_HIDE_UI] = (is_steam ? 0x1414cc501 : 0x1414cc501) + offset;
    g_Address[PATCH_DETECTION] = (is_steam ? 0x14057f1be : 0x14057f1be) + offset;
}

uintptr_t GetAddress(Address address)
{
    return g_Address[address];
}
}; // namespace gz