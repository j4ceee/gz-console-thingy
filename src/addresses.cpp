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

    g_Address[SANITY_CHECK] = (is_steam ? 0x141b60a40 : 0x141c1fbe0) + offset;
    g_Address[INST_CHARACTER_MANAGER] = (is_steam ? 0x142abed18 : 0x142bcc2c8) + offset;
    g_Address[INST_GAME_WORLD] = (is_steam ? 0x142adcb60 : 0x142bea110) + offset;
    g_Address[INST_INPUT_MANAGER] = (is_steam ? 0x1429b2558 : 0x142abfb08) + offset;
    g_Address[INST_NETWORK_PLAYER_MANAGER] = (is_steam ? 0x142adea38 : 0x142bec088) + offset;
    g_Address[INST_BASE_NETWORK_MANAGER] = (is_steam ? 0x142abe1b0 : 0x142bcb760) + offset;
    g_Address[INST_PLAYER_INFORMATION] = (is_steam ? 0x142abee30 : 0x142bcc3e0) + offset;
    g_Address[INST_PHYSICS_SYSTEM] = (is_steam ? 0x142a35da0 : 0x142b43350) + offset;
    g_Address[INST_ENVIRONMENT_GFX_MANAGER] = (is_steam ? 0x142a2b4e0 : 0x142b38a90) + offset;
    g_Address[INST_SPAWN_SYSTEM] = (is_steam ? 0x142abed20 : 0x142bcc2d0) + offset;
    g_Address[INST_WORLDTIME] = (is_steam ? 0x142abdb48 : 0x142bcb0f8) + offset;
    g_Address[INST_MAP] = (is_steam ? 0x142adfec8 : 0x142bed558) + offset;
    g_Address[INST_SOCIAL_MANAGER] = (is_steam ? 0x142abebe8 : 0x142bcc198) + offset;
    g_Address[INST_VEHICLE_MANAGER] = (is_steam ? 0x142abefe8 : 0x142bcc598) + offset;
    g_Address[INST_UI_MANAGER] = (is_steam ? 0x142abf4e8 : 0x142bcca98) + offset;
    g_Address[INST_OVERLAY_UI] = (is_steam ? 0x142adfe48 : 0x142bed630) + offset;
    g_Address[INST_CLOCK] = (is_steam ? 0x142a2b4e8 : 0x142b38a98) + offset;
    g_Address[INST_RESERVE_WORLD] = (is_steam ? 0x142abeed8 : 0x142bcc488) + offset;
    g_Address[INST_NETWORK_COMP_MANAGER] = (is_steam ? 0x142abe1e8 : 0x142bcb798) + offset;
    g_Address[INST_CAMERA_DIRECTOR] = (is_steam ? 0x142abdde8 : 0x142bcb398) + offset;
    g_Address[INST_CAMERA_MANAGER] = (is_steam ? 0x142a2b4d0 : 0x142b38a80) + offset;
    g_Address[INST_GAME_CAMERA_MANAGER] = (is_steam ? 0x142abdba0 : 0x142bcb150) + offset;
    g_Address[WND_PROC] = (is_steam ? 0x1400a4b40 : 0x1400a3430) + offset;
    g_Address[GRAPHICS_FLIP] = (is_steam ? 0x141784320 : 0x14181a860) + offset;
    g_Address[ALLOC] = (is_steam ? 0x14009ddc0 : 0x14009e230) + offset;
    g_Address[FREE] = (is_steam ? 0x14185ca84 : 0x1418f3154) + offset;
    g_Address[HASHING_FUNC] = (is_steam ? 0x140de6b00 : 0x140e2deb0) + offset;
    g_Address[FUNC_VSNPRINTF] = (is_steam ? 0x140de69b0 : 0x140e2dd60) + offset;
    g_Address[FUNC_SCRIPT_ERROR_REPORT] = (is_steam ? 0x140e7eba0 : 0x140ec5770) + offset;
    g_Address[WORLD_TO_MAP_COORDS] = (is_steam ? 0x140938830 : 0x140938510) + offset;
    g_Address[INPUT_LOST_FOCUS] = (is_steam ? 0x140101e90 : 0x140101920) + offset;
    g_Address[INPUT_GOT_FOCUS] = (is_steam ? 0x140101e30 : 0x1401018c0) + offset;
    g_Address[UPDATE_GAMEPAD] = (is_steam ? 0x1400ff4f0 : 0x1400fef80) + offset;
    g_Address[VAR_GAME_STATE_FRONTEND] = (is_steam ? 0x1428444e0 : 0x1429483f8) + offset;
    g_Address[VAR_GAME_STATE] = (is_steam ? 0x142adcb80 : 0x142bea130) + offset;
    g_Address[VAR_CLOUD_VISIBILITY] = (is_steam ? 0x141bada08 : 0x141c6cbd0) + offset;
    g_Address[VAR_FP_PLAYER_SHADOW] = (is_steam ? 0x142adf1a4 : 0x142bec85c) + offset;
    g_Address[SPAWN_SYSTEM_SPAWN] = (is_steam ? 0x140b83d90 : 0x140b91ed0) + offset;
    g_Address[SPAWN_SYSTEM_PARSE_TAGS] = (is_steam ? 0x140b67c70 : 0x140b75d80) + offset;
    g_Address[SPAWN_SYSTEM_GET_MATCHING_RES] = (is_steam ? 0x140b53ba0 : 0x140b61f60) + offset;
    g_Address[SPAWN_VEHICLE_OWNERSHIP_CALLBACK] = (is_steam ? 0x140937b80 : 0x140937860) + offset;
    g_Address[POPULATION_ADD_GROUP] = (is_steam ? 0x1408cb440 : 0x1408cb120) + offset;
    g_Address[SPAWN_ANIMAL] = (is_steam ? 0x1409064f0 : 0x1409061d0) + offset;
    g_Address[NETWORK_COMP_CREATE] = (is_steam ? 0x1404013e0 : 0x140400f50) + offset;
    g_Address[NETWORK_COMP_SEND_ADD_EVENT] = (is_steam ? 0x1409cb970 : 0x1409d4650) + offset;
    g_Address[FUNC_SET_BLACKBOARD_INT_GET_WRAPPER] = (is_steam ? 0x14047d5c0 : 0x14047d1d0) + offset;
    g_Address[FUNC_INTRO_COMPLETE] = (is_steam ? 0x140cb57d0 : 0x140cc0ba0) + offset;
    g_Address[FUNC_UPD_VIS_SHOW] = (is_steam ? 0x140ce7200 : 0x140cf2b30) + offset;
    g_Address[FUNC_IS_UI_SHOWN] = (is_steam ? 0x140cb5a60 : 0x140cc0e30) + offset;
    g_Address[CLOCK_UPDATE_GAME] = (is_steam ? 0x1400ca3e0 : 0x1400c8ce0) + offset;
    g_Address[EVENT_SCHEDULER] = (is_steam ? 0x140a02a10 : 0x140a0d4b0) + offset;
    g_Address[TELEPORT] = (is_steam ? 0x14085d210 : 0x14085cd60) + offset;
    g_Address[FAST_TRAVEL] = (is_steam ? 0x14085d000 : 0x14085cb50) + offset;
    g_Address[SET_WEATHER_PRESET_FROM_HASH] = (is_steam ? 0x140129820 : 0x1401292b0) + offset;
    g_Address[RESTORE_DYNAMIC_WEATHER] = (is_steam ? 0x140134a00 : 0x140134490) + offset;
    g_Address[WEATHER_UPDATE] = (is_steam ? 0x14012ca80 : 0x14012c510) + offset;
    g_Address[DAMAGEABLE_SET_HEALTH] = (is_steam ? 0x1406e3980 : 0x1406e3590) + offset;
    g_Address[DAMAGEABLE_RESTORE_HEALTH] = (is_steam ? 0x1406e2670 : 0x1406e2280) + offset;
    g_Address[DAMAGEABLE_SET_INVULNERABLE] = (is_steam ? 0x1406e3b00 : 0x1406e3710) + offset;
    g_Address[DAMAGEABLE_IS_INVULNERABLE] = (is_steam ? 0x1406db850 : 0x1406db460) + offset;
    g_Address[ANIMAL_HEALTH_DAMAGE] = (is_steam ? 0x1408cd2e0 : 0x1408ccfc0) + offset;
    g_Address[CHARACTER_REVIVE] = (is_steam ? 0x140624590 : 0x1406241a0) + offset;
    g_Address[CHARACTER_GHOST_MODE] = (is_steam ? 0x140626950 : 0x140626560) + offset;
    g_Address[ANIMATED_MODEL_SET_STATE] = (is_steam ? 0x1403dc320 : 0x1403dbe90) + offset;
    g_Address[DAMAGEABLE_PART_SET_UNIT_HEALTH] = (is_steam ? 0x14062d730 : 0x14062d340) + offset;
    g_Address[FIND_NETWORK_COMPONENT_CHILDREN] = (is_steam ? 0x1404044f0 : 0x140404060) + offset;
    g_Address[REQUEST_ANIMAL_HACK] = (is_steam ? 0x1409912e0 : 0x140999430) + offset;
    g_Address[REQUEST_ANIMAL_CONTROL] = (is_steam ? 0x1406492d0 : 0x140648ee0) + offset;
    g_Address[RELEASE_ANIMAL_CONTROL] = (is_steam ? 0x14065f780 : 0x14065f390) + offset;
    g_Address[GET_SIGNAL_STRENGTH] = (is_steam ? 0x14064d850 : 0x14064d460) + offset;
    g_Address[CAMERA_CHECK_COLLISION] = (is_steam ? 0x140597840 : 0x140597450) + offset;
    g_Address[DIRECTOR_PUSH_CAMERA] = (is_steam ? 0x1403a2110 : 0x1403a1c80) + offset;
    g_Address[DEEP_WATER_CACHED_HEIGHT] = (is_steam ? 0x14064be40 : 0x14064ba50) + offset;
    g_Address[GET_BACKPACK_WEIGHT] = (is_steam ? 0x14071f630 : 0x14071f2f0) + offset;
    g_Address[GET_EQUIPMENT_WEIGHT] = (is_steam ? 0x140721f40 : 0x140721c00) + offset;
    g_Address[CONSUME_DEPLOYABLES] = (is_steam ? 0x140717580 : 0x140717240) + offset;
    g_Address[CONSUME_AMMO] = (is_steam ? 0x140770ae0 : 0x1407707d0) + offset;
    g_Address[PATCH_BUILDING_UI_CHECK_1] = (is_steam ? 0x140725640 : 0x140725300) + offset;
    g_Address[PATCH_BUILDING_UI_CHECK_2] = (is_steam ? 0x140725816 : 0x1407254d6) + offset;
    g_Address[PATCH_BUILDING_CHECK_COLLISION] = (is_steam ? 0x140bbb00c : 0x140bc933c) + offset;
    g_Address[PATCH_BUILDING_CHECK_MASTER] = (is_steam ? 0x140716080 : 0x140715d40) + offset;
    g_Address[PATCH_MAP_FAST_TRAVEL_VALIDATION] = (is_steam ? 0x140c26438 : 0x140c33168) + offset;
    g_Address[PATCH_MAP_FAST_TRAVEL_BUTTON] = (is_steam ? 0x140c6e742 : 0x140c7a902) + offset;
    g_Address[PATCH_DLC_BOUNDARY] = (is_steam ? 0x140c38f57 : 0x140c45937) + offset;
    g_Address[PATCH_RESOURCE_CONSUMPTION] = (is_steam ? 0x1407460ce : 0x140745d8e) + offset;
    g_Address[PATCH_INFINITE_FUEL] = (is_steam ? 0x140d488f4 : 0x140d54184) + offset;
    g_Address[PATCH_MAP_ZOOM] = (is_steam ? 0x140c3f1ee : 0x140c4b91e) + offset;
}

uintptr_t GetAddress(Address address)
{
    return g_Address[address];
}
} // namespace gz