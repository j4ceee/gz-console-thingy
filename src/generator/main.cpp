#include <meow_hook/pattern_search.h>

#include <Windows.h>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

#pragma warning(disable : 4477)

using FindPatternResult = std::vector<std::pair<std::string, intptr_t>>;

static uintptr_t RebaseFileOffset(std::string_view game_file, const uintptr_t file_offset)
{
    auto executable_address = reinterpret_cast<intptr_t>(game_file.data());

    auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(executable_address);
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        throw std::runtime_error("Invalid DOS Signature");
    }

    auto header = reinterpret_cast<PIMAGE_NT_HEADERS>((reinterpret_cast<char*>(executable_address) + (dosHeader->e_lfanew * sizeof(char))));
    if (header->Signature != IMAGE_NT_SIGNATURE) {
        throw std::runtime_error("Invalid NT Signature");
    }

    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(header);

    for (int32_t i = 0; i < header->FileHeader.NumberOfSections; i++, section++) {
        bool executable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
        bool readable   = (section->Characteristics & IMAGE_SCN_MEM_READ) != 0;

        if (file_offset >= section->PointerToRawData
            && file_offset <= (section->PointerToRawData + section->SizeOfRawData)) {
            return static_cast<uint64_t>(0x140000000) + file_offset + ((section->VirtualAddress - section->PointerToRawData));
        }
    }

    std::cout << "Failed to rebase.\n";
    return 0;
}

std::string sanity_check = "4C 89 ? ? ? ? ? 48 8D ? ? ? ? ? 48 89 ? ? ? ? ? 4C 89 ? ? ? ? ? C7";

void FindPattern(const char* name, std::string_view game_file, FindPatternResult& result, const std::function<uintptr_t()>& func)
{
    try { meow_hook::pattern(sanity_check, game_file).count(1).get(0); } catch (...) {}

    uintptr_t addr = 0;

    try {
        addr = func();
    } catch (...) {
        std::cout << "ERROR! ";
    }

    result.emplace_back(name, addr);
    std::cout << std::setw(40) << std::left << name << std::right << "0x" << std::hex << addr << "\n";
}

intptr_t rebase(std::string_view game_file, intptr_t addr)
{
    return RebaseFileOffset(game_file, addr - reinterpret_cast<intptr_t>(game_file.data()));
}

// HACK: work around for pattern::match::add_disp
meow_hook::pattern::match disp_rebase(std::string_view game_file, meow_hook::pattern::match& match)
{
    auto addr = RebaseFileOffset(game_file, match.addr() - reinterpret_cast<intptr_t>(game_file.data()));
    return meow_hook::pattern::match{addr + *reinterpret_cast<int32_t*>(match.addr())}.adjust(4);
}

FindPatternResult Generate(const char* name, const char* exepath)
{
    using namespace meow_hook;

    FindPatternResult result;

    std::fstream is(exepath, std::ios::in | std::ios::binary);
    is.seekg(0, std::ios::end);
    size_t data_size = is.tellg();
    is.seekg(0, std::ios::beg);
    std::unique_ptr<char[]> data(new char[data_size]);
    is.read(data.get(), data_size);

    std::string_view game_file(data.get(), data_size);

    std::cout << name << "\n";
    std::cout << "-----------\n";

    // clang-format off

    FindPattern("SANITY_CHECK", game_file, result, [&] {
        auto match = pattern(sanity_check, game_file)
            .count(1)
            .get(0)
            .adjust(10);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    // -- instances --

    FindPattern("INST_CHARACTER_MANAGER", game_file, result, [&] {
        auto match = pattern("48 8B 05 ? ? ? ? 48 85 C0 74 08 48 8B 80 ? ? ? ?", game_file).count(1).get(0).adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_GAME_WORLD", game_file, result, [&] {
        auto match = pattern("48 8B 05 ? ? ? ? 8B 50 28 33 C0 48 89", game_file).count(1).get(0).adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_INPUT_MANAGER", game_file, result, [&] {
        auto match = pattern("E8 ? ? ? ? ? ? 49 ? ? 48 89 ? ? ? ? ? 33 D2", game_file).count(1).get(0).adjust(13);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_NETWORK_PLAYER_MANAGER", game_file, result, [&] {
        auto match = pattern("48 ? ? ? ? ? ? 41 ? ? ? ? ? 48 ? ? ? 0f ? ? ? ? ? ? 48 ? ? 38", game_file).count(1).get(0).adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_BASE_NETWORK_MANAGER", game_file, result, [&] {
        auto match = pattern("FF ? ? 90 48 ? ? ? ? ? ? 48 ? ? ? 48 ? ? 74 ? 0F ? ? ? 88", game_file).count(1).get(0).adjust(7);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_PLAYER_INFORMATION", game_file, result, [&] {
        auto match = pattern("48 8B F2 48 8B 0D ? ? ? ? 48 81 C1 ? ? ? ? E8", game_file).count(1).get(0).adjust(6);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_PHYSICS_SYSTEM", game_file, result, [&] {
        auto match = pattern("57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B F9 4C 8B ? ? 8B 81 ? ? ? ? 3D", game_file)
            .count(1)
            .get(0)
            .adjust(8);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_ENVIRONMENT_GFX_MANAGER", game_file, result, [&] {
        auto match = pattern("48 8B 0D ? ? ? ? 8B D0 E8 ? ? ? ? 41 B8 ? ? ? ? 49", game_file)
            .count(1)
            .get(0)
            .adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_SPAWN_SYSTEM", game_file, result, [&] {
        auto match = pattern("48 89 73 ? 48 89 73 ? 48 8B 05 ? ? ? ? FF 88 ? ? ? ?", game_file)
            .count(1)
            .get(0)
            .adjust(11);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_WORLDTIME", game_file, result, [&] {
        auto match = pattern("73 16 48 8B 05 ? ? ? ?", game_file).count(1).get(0).adjust(5);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_MAP", game_file, result, [&] {
        auto match = pattern("41 B8 ? ? ? ? 48 8D ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0 74", game_file)
            .count(1)
            .get(0)
            .adjust(14);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_SOCIAL_MANAGER", game_file, result, [&] {
        auto match = pattern("0F ? ? F3 0F ? ? ? 48 ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 45 ? ? ? 45", game_file)
            .count(1)
            .get(0)
            .adjust(15);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_VEHICLE_MANAGER", game_file, result, [&] {
        auto match = pattern("4C ? ? ? 48 ? ? ? 48 ? ? ? ? ? ? E8 ? ? ? ? 48 8B F8 41 8B", game_file)
            .count(1)
            .get(0)
            .adjust(11);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_UI_MANAGER", game_file, result, [&] {
        auto match = pattern("CC 48 ? ? ? 48 8B 0D ? ? ? ? 48 ? ? ? ? C7 ? ? ? ? ? ? ? E8", game_file)
            .count(1)
            .get(0)
            .adjust(8);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_OVERLAY_UI", game_file, result, [&] {
        auto match = pattern("48 ? ? ? ? ? ? F3 ? ? ? ? ? F3 ? ? ? ? ? 0F ? ? 0F ? ? E8 ? ? ? ? 0F ? ? ? ? 48 ? ? ? ? 48", game_file)
            .count(1)
            .get(0)
            .adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_CLOCK", game_file, result, [&] {
        auto match = pattern("48 ? ? ? ? ? ? E8 ? ? ? ? F3 ? ? ? ? ? ? ? 0F 57 C9 48", game_file)
            .count(1)
            .get(0)
            .adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_RESERVE_WORLD", game_file, result, [&] {
        auto match = pattern("48 8B ? ? ? ? ? 48 ? ? ? 4C ? ? ? 4C ? ? ? ? 45", game_file)
            .count(1)
            .get(0)
            .adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_NETWORK_COMP_MANAGER", game_file, result, [&] {
        auto match = pattern("8B ? ? 48 8B ? ? ? ? ? E8 ? ? ? ? 48 ? ? ? ? 48 ? ? ? ? ? ? 48 ? ? ? 48 85 ? ? ? 0F ? ? ? 33", game_file)
            .count(1)
            .get(0)
            .adjust(6);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_CAMERA_DIRECTOR", game_file, result, [&] {
        auto match = pattern("48 ? ? ? ? ? ? 48 ? ? 48 ? ? ? ? 48 ? ? ? ? 48 ? ? ? 5F E9 B5 ? ? ? 48", game_file)
            .count(1)
            .get(0)
            .adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_CAMERA_MANAGER", game_file, result, [&] {
       auto match = pattern("48 83 EC 38 48 8B 05 ? ? ? ? 48 8B 88", game_file)
           .count(1)
           .get(0)
           .adjust(7);
       return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_GAME_CAMERA_MANAGER", game_file, result, [&] {
       auto match = pattern("AF F0 5B 4C 48 8B 0D", game_file)
           .count(1)
           .get(0)
           .adjust(7);
       return disp_rebase(game_file, match).as<uintptr_t>();
    });

    // -- base functions --

    FindPattern("WND_PROC", game_file, result, [&] {
        auto match = pattern("E9 ? ? ? ? 48 ? ? ? ? ? ? ? 48 8D 05 ? ? ? ? 48 ? ? ? 4C ? ? ? 48", game_file)
            .count(1)
            .get(0)
            .adjust(16);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("GRAPHICS_FLIP", game_file, result, [&] {
        auto match = pattern("48 8B C8 E8 ? ? ? ? 90 48 8B 8F ? ? ? ? E8 ? ? ? ? 90 48 8B CB E8", game_file)
            .count(1)
            .get(0)
            .adjust(9 + 7)  // skip past first call pattern (9 bytes) then adjust to second call (7 bytes)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("ALLOC", game_file, result, [&] {
        // MOV RCX,RDI; CALL; MOV [RSI],RAX; MOV [RSI+8],RAX
        auto match = pattern("48 8B CF E8 ? ? ? ? 48 89 06 48 89 46 08", game_file)
            .get(0)
            .adjust(3)  // offset to CALL instruction
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("FREE", game_file, result, [&] {
        // CALL; MOV EDX; MOV RCX; CALL FREE; NOP; LEA RCX; CALL; MOV
        auto match = pattern("E8 ? ? ? ? BA ? ? 00 00 48 8B 4C 24 ? E8 ? ? ? ? 90 48 8D 4D ? E8 ? ? ? ? 48 8B", game_file)
            .count(1)
            .get(0)
            .adjust(15)  // offset to FREE CALL
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("HASHING_FUNC", game_file, result, [&] {
        auto match = pattern("49 8B ? FF ? ? ? ? ? ? ? ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 89 44 24 ? 48 8B ? ?", game_file)
            .count(1)
            .get(0)
            .adjust(23);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("FUNC_VSNPRINTF", game_file, result, [&] {
        auto match = pattern("74 ? 48 8D ? ? 48 89 0E 4D 8B CE 4C 8B C5 48 8B D7 48 8B CB E8 ? ? ? ?", game_file)
            .count(1)
            .get(0)
            .adjust(21)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("FUNC_SCRIPT_ERROR_REPORT", game_file, result, [&] {
        auto match = pattern("4C 8B C3 48 8D 15 ? ? ? ? 48 8B CE E8 ? ? ? ? E9 ? ? ? ? F2 0F", game_file)
            .count(1)
            .get(0)
            .adjust(13)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("WORLD_TO_MAP_COORDS", game_file, result, [&] {
        auto match = pattern("F3 ? ? ? ? ? ? ? E8 ? ? ? ? 48 ? ? ? 4C ? ? ? 48", game_file)
            .count(1)
            .get(0)
            .adjust(8)
            .extract_call();
        return rebase(game_file, match);
    });

    // -- input --

    FindPattern("INPUT_LOST_FOCUS", game_file, result, [&] {
        auto match = pattern("E8 ? ? ? ? 48 8B ? ? ? ? ? 48 85 C9 ? ? 48 ? ? E8 ? ? ? ? 48 ? ? 48", game_file)
            .count(1)
            .get(0)
            .adjust(20)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("INPUT_GOT_FOCUS", game_file, result, [&] {
        auto match = pattern("E8 ? ? ? ? 48 8B ? ? ? ? ? 48 85 C9 ? ? 48 ? ? E8 ? ? ? ? ? ? 40", game_file)
            .count(1)
            .get(0)
            .adjust(20)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("UPDATE_GAMEPAD", game_file, result, [&] {
        auto match = pattern("0F ? ? ? 44 0F ? ? 41 ? ? 48 ? ? E8", game_file)
            .count(1)
            .get(0)
            .adjust(14)
            .extract_call();
        return rebase(game_file, match);
    });

    // -- variables --

    FindPattern("VAR_GAME_STATE_FRONTEND", game_file, result, [&] {
        auto match = pattern("8B ? ? ? ? ? 83 ? ?  74 ? 83 ? ? 0f ? ? ? ? ? 83 ? ? 0f ? ? ? ? ? 83 ? ? 74", game_file)
            .count(1)
            .get(0)
            .adjust(2);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("VAR_GAME_STATE", game_file, result, [&] {
        auto match = pattern("8B ? ? ? ? ? 83 ? ? 74 ? 83 ? ? 0f ? ? ? ? ? 83 ? ? 74", game_file)
            .count(1)
            .get(0)
            .adjust(2);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("VAR_CLOUD_VISIBILITY", game_file, result, [&] {
        auto match = pattern("F3 0F 59 ? ? ? ? ? F3 0F 5C C8 F3 0F 5F CC F3", game_file)
            .count(1)
            .get(0)
            .adjust(4);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("VAR_FP_PLAYER_SHADOW", game_file, result, [&] { // boolean value for first-person player shadow toggle
        auto match = pattern("0F ? ? ? ? ? 80 ? ? ? ? ? ? 0F ? ? ? ? ? ? ? ? E8 ? ? ? ? 8B D0", game_file)
            .count(1)
            .get(0)
            .adjust(8);
        // adjust disp_rebase for the extra trailing byte
        auto intermediate = disp_rebase(game_file, match);
        return (intermediate.as<uintptr_t>() + 1);  // add the missing byte
    });

    // -- spawn system --

    FindPattern("SPAWN_SYSTEM_SPAWN", game_file, result, [&] {
        auto match = pattern("8B 97 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? EB ? 80 B9 ? ? ? ? ? 74", game_file)
            .count(1)
            .get(0)
            .adjust(13)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("SPAWN_SYSTEM_PARSE_TAGS", game_file, result, [&] {
        auto match = pattern("4C 8D 4C ? ? C7 44 ? ? ? ? ? ? 4C 8D ? ? ? E8 ? ? ? ? 85 C0 74", game_file)
            .count(1)
            .get(0)
            .adjust(18)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("SPAWN_SYSTEM_GET_MATCHING_RES", game_file, result, [&] {
        auto match = pattern("E8 ? ? ? ? 85 C0 74 ? 4D 8B CF", game_file)
            .count(1)
            .get(0)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("SPAWN_VEHICLE_OWNERSHIP_CALLBACK", game_file, result, [&] {
        auto match = pattern("45 33 E4 4C ? ? ? ? ? ? 44 38 A1 ? ? ? ? 74 ? 45 33 C9", game_file)
            .count(1)
            .get(0)
            .adjust(6);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("POPULATION_ADD_GROUP", game_file, result, [&] {
        auto match = pattern("4C ? ? ? ? ? ? ? 88 ? ? ? E8 ? ? ? ? 48 ? ? ? 5B C3", game_file)
            .count(1)
            .get(0)
            .adjust(12)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("SPAWN_ANIMAL", game_file, result, [&] {
        auto match = pattern("48 ? ? 49 ? ? E8 ? ? ? ? 48 ? ? ? 80", game_file)
            .count(1)
            .get(0)
            .adjust(6)
            .extract_call();
        return rebase(game_file, match);
    });

    // -- network stuff --

    FindPattern("NETWORK_COMP_CREATE", game_file, result, [&] {
        auto match = pattern("8B ? ? ? 89 ? ? ? 8B ? ? ? 89 ? ? ? E8 ? ? ? ? 48 ? ? ? C3", game_file)
            .count(1)
            .get(0)
            .adjust(16)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("NETWORK_COMP_SEND_ADD_EVENT", game_file, result, [&] {
        auto match = pattern("40 55 48 8D ? ? ? ? ? ? 48 81 ? ? ? ? ? 48 C7 ? ? ? ? ? ? ? 48 89 ? ? ? ? ? ? 48 8B ? ? ? ? ? 48 33 C4 48 89 ? ? ? ? ? 48 8B ? 44", game_file)
            .count(1)
            .get(0)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    // -- random bullshit go --

    FindPattern("FUNC_SET_BLACKBOARD_INT_GET_WRAPPER", game_file, result, [&] {
        auto match = pattern("45 ? ? 41 ? 01 ? 71 1E DF 41 E8 ? ? ? ? 48 ? FF FF FF FF FF 00 00 00 48 89", game_file)
            .count(1)
            .get(0)
            .adjust(11)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("FUNC_INTRO_COMPLETE", game_file, result, [&] {
        auto match = pattern("75 ? 48 ? ? ? ? ? ? E8 ? ? ? ? 84 C0 75 ? 8B", game_file)
            .count(1)
            .get(0)
            .adjust(9)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("FUNC_UPD_VIS_SHOW", game_file, result, [&] {
        auto match = pattern("0F ? ? ? ? ? ? 48 ? ? E8 ? ? ? ? 48 FF C7 48 ? ? ? 48 3B FE 75", game_file)
            .count(1)
            .get(0)
            .adjust(10)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("FUNC_IS_UI_SHOWN", game_file, result, [&] {
        auto match = pattern("F1 11 B5 97 E8 ? ? ? ? 84", game_file)
            .count(1)
            .get(0)
            .adjust(4)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("CLOCK_UPDATE_GAME", game_file, result, [&] {
        auto match = pattern("48 ? ? ? ? ? ? E8 ? ? ? ? F3 ? ? ? ? ? ? ? 0F 57 C9 48", game_file)
            .count(1)
            .get(0)
            .adjust(7)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("EVENT_SCHEDULER", game_file, result, [&] {
        auto match = pattern("48 8B ? ? 48 85 FF 74 ? 48 8B CF E8 ? ? ? ? 48 8B ? ? 0F B6 ? ? ?", game_file)
            .count(1)
            .get(0)
            .adjust(13);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    // vftable function of CGameWorld at index 21 (Ghidra)
    // teleport function used when falling into water
    FindPattern("TELEPORT", game_file, result, [&] {
        auto match = pattern("48 89 5c ? ? 48 89 6c ? ? 48 89 74 ? ? 48 89 7c ? ? ? ? 48 83 ? ? 48 8b 05 ? ? ? ? 33 db 49 8b e8", game_file)
            .count(1)
            .get(0)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    // vftable function of CGameWorld at index 19 (Ghidra)
    // fast travel function used by map & on dlc islands
    FindPattern("FAST_TRAVEL", game_file, result, [&] {
        auto match = pattern("F3 0F ? ? ? ? ? ? ? F3 0F ? ? ? ? 88 ? ? ? E8 ? ? ? ? 48", game_file)
            .count(1)
            .get(0)
            .adjust(19)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("SET_WEATHER_PRESET_FROM_HASH", game_file, result, [&] {
        auto match = pattern("48 8B CF E8 ? ? ? ? 48 8B 0D ? ? ? ? 8B D0 E8 ? ? ? ? 84 C0 75", game_file)
            .count(1)
            .get(0)
            .adjust(17)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("RESTORE_DYNAMIC_WEATHER", game_file, result, [&] {
        auto match = pattern("48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 33 C0 EB ? 3B 1D", game_file)
            .count(1)
            .get(0)
            .adjust(7)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("WEATHER_UPDATE", game_file, result, [&] {
       auto match = pattern("85 D2 ? ? E8 ? ? ? ? 44 8B 83 ? ? ? ? 45", game_file)
           .count(1)
           .get(0)
           .adjust(4)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("DAMAGEABLE_SET_HEALTH", game_file, result, [&] {
       auto match = pattern("48 ? ? ? ? 48 ? ? ? ? 57 48 ? ? ? 48 ? ? 41 ? ? ? 8B ? ? ? ? ? 49", game_file)
           .count(1)
           .get(0)
           .as<uintptr_t>();
       return rebase(game_file, match);
    });

    FindPattern("DAMAGEABLE_RESTORE_HEALTH", game_file, result, [&] {
       auto match = pattern("40 ? 48 ? ? ? 66 ? ? ? ? ? ? ? ? ? ? 4C ? ? 45 ? ?", game_file)
           .count(1)
           .get(0)
           .as<uintptr_t>();
       return rebase(game_file, match);
    });

    FindPattern("DAMAGEABLE_SET_INVULNERABLE", game_file, result, [&] {
       auto match = pattern("83 ? ? ? ? ? ? ? ? 48 ? ? E8 ? ? ? ? 41 ? ? ? ? ? ? ? 48 ? ? E8", game_file)
           .count(1)
           .get(0)
           .adjust(28)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("DAMAGEABLE_IS_INVULNERABLE", game_file, result, [&] {
       auto match = pattern("CC 48 ? ? ? BA ? ? ? ? E8 ? ? ? ? ? ? 0F ? ? 48 ? ?  ? C3", game_file)
           .count(1)
           .get(0)
           .adjust(10)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("ANIMAL_HEALTH_DAMAGE", game_file, result, [&] {
       auto match = pattern("0F ? ? E8 ? ? ? ? 49 ? ? ? E8 ? ? ? ? 49", game_file)
           .count(1)
           .get(0)
           .adjust(3)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("CHARACTER_REVIVE", game_file, result, [&] {
       auto match = pattern("B2 ? 48 ? ? E8 ? ? ? ? EB ? C7 ? ? ? ?", game_file)
           .count(1)
           .get(0)
           .adjust(5)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("CHARACTER_GHOST_MODE", game_file, result, [&] {
       auto match = pattern("83 ? ? ? ? ? ? ? ? 48 ? ? E8 ? ? ? ? 41 ? ? ? ? ? ? ? 48 ? ? E8", game_file)
           .count(1)
           .get(0)
           .adjust(12)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("ANIMATED_MODEL_SET_STATE", game_file, result, [&] {
       auto match = pattern("48 8B C8 E8 ? ? ? ? 48 8B CD E8", game_file)
           .count(1)
           .get(0)
           .adjust(3)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("DAMAGEABLE_PART_SET_UNIT_HEALTH", game_file, result, [&] {
       auto match = pattern("48 ? ? ? E8 ? ? ? ? 48 ? ? ? 48 ? ? 75 ? 48 ? ? ? ? 49", game_file)
           .count(1)
           .get(0)
           .adjust(4)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("FIND_NETWORK_COMPONENT_CHILDREN", game_file, result, [&] {
       auto match = pattern("E8 ? ? ? ? 4C ? ? 45 ? ? 48 ? ? 0F ? ? ? ? ? 8B", game_file)
           .count(1)
           .get(0)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("REQUEST_ANIMAL_HACK", game_file, result, [&] {
       auto match = pattern("E8 ? ? ? ? 89 ? ? ? ? ? 48 ? ? E8 ? ? ? ? 84 ?", game_file)
           .count(1)
           .get(0)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("REQUEST_ANIMAL_CONTROL", game_file, result, [&] {
       auto match = pattern("44 0F ? ? ? ? ? ? 48 ? ? ? ? ? ? 48 8B CF E8 ? ? ? ? 44", game_file)
           .count(1)
           .get(0)
           .adjust(18)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("RELEASE_ANIMAL_CONTROL", game_file, result, [&] {
       auto match = pattern("E8 ? ? ? ? ? ? F3 0F ? ? ? ? ? ? F3 41 ? ? ? ? F3 0F ? ? ? ? ? ? 48", game_file)
           .count(1)
           .get(0)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("GET_SIGNAL_STRENGTH", game_file, result, [&] {
       auto match = pattern("E8 ? ? ? ? 48 ? ? E8 ? ? ? ? 0F ? ? 48 ? ? ? ? ? ? 48 ? ? ? ? E8", game_file)
           .count(1)
           .get(0)
           .adjust(8)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("CAMERA_CHECK_COLLISION", game_file, result, [&] {
       auto match = pattern("E8 ? ? ? ? F3 0F ? ? ? ? ? ? 41 ? ? ? ? ? 48 ? ? ? ? ? ? 48", game_file)
           .count(1)
           .get(0)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("DIRECTOR_PUSH_CAMERA", game_file, result, [&] {
        auto match = pattern("48 ? ? ? ? ? ? 48 ? ? 48 ? ? ? ? 48 ? ? ? ? 48 ? ? ? 5F E9 B5 ? ? ? 48", game_file)
            .count(1)
            .get(0)
            .adjust(25)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("DEEP_WATER_CACHED_HEIGHT", game_file, result, [&] {
       auto match = pattern("48 83 EC 38 48 8B 05 ? ? ? ? 48 8B 88", game_file)
           .count(1)
           .get(0)
           .as<uintptr_t>();
       return rebase(game_file, match);
    });

    FindPattern("GET_BACKPACK_WEIGHT", game_file, result, [&] {
       auto match = pattern("48 ? ? F3 ? ? ? ? ? ? ? E8 ? ? ? ? F3 ? ? ? 0F ? ? ? ? 0F", game_file)
           .count(1)
           .get(0)
           .adjust(11)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("GET_EQUIPMENT_WEIGHT", game_file, result, [&] {
       auto match = pattern("48 ? ? 74 ? 48 ? ? E8 ? ? ? ? F3 ? ? ? 40", game_file)
           .count(1)
           .get(0)
           .adjust(8)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("CONSUME_DEPLOYABLES", game_file, result, [&] {
        auto match = pattern("E8 ? ? ? ? 45 33 C0 0F B6 ? ? ? ? ? 8B ? ? ? ? ? E8 ? ? ? ? 48", game_file)
            .count(1)
            .get(0)
            .adjust(0)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("CONSUME_AMMO", game_file, result, [&] {
        auto match = pattern("85 D2 0F ? ? ? ? ? 4C ? ? 41 ? 48 83 ? ? 49 C7 43 ? ? ? ? ? 49 89 ? ? 49 89 ? ? 49 89 ? ? 49 89 ? ? 44", game_file)
            .count(1)
            .get(0)
            .adjust(0)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    FindPattern("BUILDING_CAN_PLACE_BUILDING", game_file, result, [&] {
        auto match = pattern("48 8B CF E8 ? ? ? ? 8B ? ? ? ? ? 84 ? 0F", game_file)
            .count(1)
            .get(0)
            .adjust(3)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("BUILDING_ITEM_GET_SPAWN_LOCATION", game_file, result, [&] {
        auto match = pattern("48 8B CF E8 ? ? ? ? C6 ? ? ? ? C6 ? ? ? ? C6 ? ? ? ? 41", game_file)
            .count(1)
            .get(0)
            .adjust(3)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("BUILDING_ALIGN_TO_GRID", game_file, result, [&] {
        auto match = pattern("49 8B CA E8 ? ? ? ? 84 C0 74 ? ? ? ? ? 32", game_file)
            .count(1)
            .get(0)
            .adjust(3)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("BUILDING_GRID_ADD_BUILDING", game_file, result, [&] {
        auto match = pattern("E8 ? ? ? ? 84 C0 0F ? ? ? ? ? 41 ? ? ? ? ? ? ? ? 41 ? ? 49", game_file)
            .count(1)
            .get(0)
            .adjust(0)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("BUILDING_GRID_COMPUTE_FOOTPRINT_BOUNDS", game_file, result, [&] {
        auto match = pattern("48 8B CF E8 ? ? ? ? 84 C0 0F ? ? ? ? ? 44 ? ? ? 44", game_file)
            .count(1)
            .get(0)
            .adjust(3)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("MAP_ALLOWED_IN_REGION", game_file, result, [&] {
        auto match = pattern("89 ? ? ? E8 ? ? ? ? 84 ? ? ? 48 ? ? ? ? ? ? 40", game_file)
            .count(1)
            .get(0)
            .adjust(4)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("MAP_ON_MANAGE_INPUT_ICONS", game_file, result, [&] {
        auto match = pattern("49 8B CF E8 ? ? ? ? C6 ? ? ? ? ? ? 4C ? ? ? 48", game_file)
            .count(1)
            .get(0)
            .adjust(3)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("MAP_FAST_TRAVEL", game_file, result, [&] {
        auto match = pattern("48 8B D3 E8 ? ? ? ? E9 ? ? ? ? 81 ? ? ? ? ? 0F", game_file)
            .count(1)
            .get(0)
            .adjust(3)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("UI_GET_UINT64", game_file, result, [&] {
        auto match = pattern("48 8B CA E8 ? ? ? ? 4C ? ? 48 ? ? ? 0F", game_file)
            .count(1)
            .get(0)
            .adjust(3)
            .extract_call();
        return rebase(game_file, match);
    });

    // ++ BYTE PATCHES ++
    // (these are addresses of single bytes to patch, not functions or variables)

    // resource cheat
    FindPattern("PATCH_RESOURCE_CONSUMPTION", game_file, result, [&] { // reduces resources after building placement / crafting
        auto match = pattern("2B D7 41 ? ? ? 49 8B ?", game_file).count(1).get(0).adjust(0).as<uintptr_t>();
        return rebase(game_file, match);
    });

    // fuel at 100% cheat (take maximum of current fuel and max fuel)
    FindPattern("PATCH_INFINITE_FUEL", game_file, result, [&] { // 5D to 5F -> MAXSS XMM7, MaxFuel
        auto match = pattern("F3 0F 5F FE F3 0F 5D 3D ? ? ? ? F3 0F 11 BF", game_file)
            .count(1)
            .get(0)
            .adjust(4)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    // disable map max zoom
    FindPattern("PATCH_MAP_ZOOM", game_file, result, [&] {
        auto match = pattern("F3 0F 5F ? ? ? ? ? F3 0F 5D ? ? ? ? ? F3 0F 11 ? ? ? ? ? 48 89", game_file)
            .count(1)
            .get(0)
            .adjust(8)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    // clang-format on

    std::cout << "\n\n";

    return result;
}

void WriteHeader(const std::filesystem::path& path, FindPatternResult& addresses)
{
    std::ofstream stream(path);

    stream << "#pragma once\n\n";
    stream << "#include <cstdint>\n\n";

    stream << "namespace gz\n";
    stream << "{\n";

    stream << "enum Address {\n";
    for (const auto& address : addresses) {
        stream << "    " << address.first << ",\n";
    }
    stream << "    COUNT,\n";
    stream << "};\n\n";

    stream << "void InitAddresses(bool is_steam);\n";
    stream << "uintptr_t GetAddress(Address address);\n";

    stream << "}; // namespace gz";
}

void WriteSource(const std::filesystem::path& path, FindPatternResult& steam_addresses,
                 FindPatternResult& xbox_addresses)
{
    std::ofstream stream(path);

    stream << "#include \"FW1FontWrapper.h\"\n\n";
    stream << "#include \"addresses.h\"\n\n";

    stream << "namespace gz\n";
    stream << "{\n";
    stream << "static uintptr_t g_Address[Address::COUNT] = {};\n\n";

    stream << "void InitAddresses(bool is_steam)\n";
    stream << "{\n";
    // original code assumes a base address of 0x140000000
    // generation zero uses ASLR so we need to calculate the offset from the actual base address at runtime
    stream << "    HMODULE hModule = GetModuleHandle(\"GenerationZero_F.exe\");\n";
    stream << "    uintptr_t base = (uintptr_t)hModule;\n";
    stream << "    uintptr_t offset = base - 0x140000000;\n\n";

    for (int i = 0; i < steam_addresses.size(); ++i) {
        const std::string& name       = steam_addresses[i].first;
        const uintptr_t    steam_addr = steam_addresses[i].second;
        const uintptr_t    xbox_addr  = xbox_addresses[i].second;

        stream << "    g_Address[" << name << "] = (is_steam ? 0x" << std::hex << steam_addr << " : 0x" << std::hex << xbox_addr << ") + offset;\n";
    }

    stream << "}\n\n";

    stream << "uintptr_t GetAddress(Address address)\n";
    stream << "{\n";
    stream << "    return g_Address[address];\n";
    stream << "}\n";

    stream << "} // namespace gz";
}

int main()
{
    auto steam_addresses = Generate("Steam", R"(C:\Program Files (x86)\Steam\steamapps\common\GenerationZero\GenerationZero_F.exe)");
    auto xbox_addresses  = Generate("Microsoft Store", R"(C:\XboxGames\Generation Zero\Content\GenerationZero_F.exe)");

    assert(steam_addresses.size() == xbox_addresses.size());

    char tmp[MAX_PATH] = {0};
    GetModuleFileName(nullptr, tmp, sizeof(tmp));

    std::filesystem::path path(tmp);
    std::filesystem::path base_path = path.parent_path().parent_path().parent_path().parent_path() / "src";
    std::cout << "Writing to " << base_path << "\n";
    WriteHeader(base_path / "addresses.h", steam_addresses);
    WriteSource(base_path / "addresses.cpp", steam_addresses, xbox_addresses);
    return 0;
}
