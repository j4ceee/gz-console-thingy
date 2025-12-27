#include <meow_hook/pattern_search.h>

#include <Windows.h>

#include <assert.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

#pragma warning(disable : 4477)

using FindPatternResult = std::vector<std::pair<std::string, intptr_t>>;

static uintptr_t RebaseFileOffset(std::string_view game_file, uintptr_t file_offset)
{
    auto executable_address = reinterpret_cast<intptr_t>(game_file.data());

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)(executable_address);
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        throw std::runtime_error("Invalid DOS Signature");
    }

    PIMAGE_NT_HEADERS header = (PIMAGE_NT_HEADERS)(((char*)executable_address + (dosHeader->e_lfanew * sizeof(char))));
    if (header->Signature != IMAGE_NT_SIGNATURE) {
        throw std::runtime_error("Invalid NT Signature");
    }

    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(header);

    for (int32_t i = 0; i < header->FileHeader.NumberOfSections; i++, section++) {
        bool executable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
        bool readable   = (section->Characteristics & IMAGE_SCN_MEM_READ) != 0;

        if (file_offset >= section->PointerToRawData
            && file_offset <= (section->PointerToRawData + section->SizeOfRawData)) {
            return uint64_t(0x140000000) + file_offset + ((section->VirtualAddress - section->PointerToRawData));
        }
    }

    std::cout << "Failed to rebase.\n";
    return 0;
}

void FindPattern(const char* name, FindPatternResult& result, std::function<uintptr_t()> func)
{
    uintptr_t addr = 0;

    try {
        addr = func();
    } catch (...) {
        std::cout << "ERROR! ";
    }

    result.push_back({name, addr});
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

    FindPattern("SANITY_CHECK", result, [&] {
        auto match = pattern("48 8D 05 ? ? ? ? 48 89 81 ? ? ? ? 4C 89 B9 ? ? ? ? C7 81 ? ? ? ? ? ? ? ?", game_file).count(1).get(0).adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_CHARACTER_MANAGER", result, [&] {
        auto match = pattern("48 8B 05 ? ? ? ? 48 85 C0 74 08 48 8B 80 ? ? ? ?", game_file).count(1).get(0).adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_GAME_WORLD", result, [&] {
        auto match = pattern("48 8B 05 ? ? ? ? 8B 50 28 33 C0 48 89", game_file).count(1).get(0).adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_INPUT_MANAGER", result, [&] {
        auto match = pattern("48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8D 45 B0", game_file).count(1).get(0).adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_NETWORK_PLAYER_MANAGER", result, [&] {
        auto match = pattern("48 8B ? ? ? ? ? 48 8B ? ? 48 85 ? 0F 84 ? ? ? ? 4C ? ? ? ? ? ? 48 8B ? ? ? ? ? 48 85 ? 74 ? B8", game_file).count(1).get(0).adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_PLAYER_INFORMATION", result, [&] {
        auto match = pattern("48 8B F2 48 8B 0D ? ? ? ? 48 81 C1 ? ? ? ? E8", game_file).count(1).get(0).adjust(6);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_PHYSICS_SYSTEM", result, [&] {
        auto match = pattern("57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B F9 4C 8B ? ? 8B 81 ? ? ? ? 3D", game_file)
            .count(1)
            .get(0)
            .adjust(8);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_ENVIRONMENT_GFX_MANAGER", result, [&] {
        auto match = pattern("48 8B 0D ? ? ? ? 8B D0 E8 ? ? ? ? 41 B8 ? ? ? ? 49", game_file)
            .count(1)
            .get(0)
            .adjust(3);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });


    FindPattern("INST_SPAWN_SYSTEM", result, [&] {
        auto match = pattern("48 89 73 ? 48 89 73 ? 48 8B 05 ? ? ? ? FF 88 ? ? ? ?", game_file)
            .count(1)
            .get(0)
            .adjust(11);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_WORLDTIME", result, [&] {
        auto match = pattern("73 16 48 8B 05 ? ? ? ?", game_file).count(1).get(0).adjust(5);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_MAP", result, [&] {
        auto match = pattern("41 B8 ? ? ? ? 48 8D ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0 74", game_file)
            .count(1)
            .get(0)
            .adjust(14);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("INST_VEHICLE_MANAGER", result, [&] {
        auto match = pattern("4C ? ? ? 48 ? ? ? 48 ? ? ? ? ? ? E8 ? ? ? ? 48 8B F8 41 8B", game_file)
            .count(1)
            .get(0)
            .adjust(11);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("WND_PROC", result, [&] {
        auto match = pattern("40 53 55 56 57 48 83 EC ? 48 8B ? ? ? ? ? 48 33 C4 48 89 44 ? ? 49 8B D9", game_file)
            .count(1)
            .get(0)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    FindPattern("GRAPHICS_FLIP", result, [&] {
        auto match = pattern("48 8B C8 E8 ? ? ? ? 90 48 8B 8F ? ? ? ? E8 ? ? ? ? 90 48 8B CB E8", game_file)
            .count(1)
            .get(0)
            .adjust(9 + 7)  // skip past first call pattern (9 bytes) then adjust to second call (7 bytes)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("VAR_GAME_STATE", result, [&] {
        auto match = pattern("8B 05 ? ? ? ? 83 C0 F8", game_file).count(1).get(0).adjust(2);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("VAR_CLOUD_VISIBILITY", result, [&] {
        auto match = pattern("F3 0F 59 ? ? ? ? ? F3 0F 5C C8 F3 0F 5F CC F3", game_file)
            .count(1)
            .get(0)
            .adjust(4);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("ALLOC", result, [&] {
        // MOV RCX,RDI; CALL; MOV [RSI],RAX; MOV [RSI+8],RAX
        auto match = pattern("48 8B CF E8 ? ? ? ? 48 89 06 48 89 46 08", game_file)
            .get(0)
            .adjust(3)  // offset to CALL instruction
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("FREE", result, [&] {
        // CALL; MOV EDX; MOV RCX; CALL FREE; NOP; LEA RCX; CALL; MOV
        auto match = pattern("E8 ? ? ? ? BA ? ? 00 00 48 8B 4C 24 ? E8 ? ? ? ? 90 48 8D 4D ? E8 ? ? ? ? 48 8B", game_file)
            .count(1)
            .get(0)
            .adjust(15)  // offset to FREE CALL
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("SPAWN_SYSTEM_SPAWN", result, [&] {
        auto match = pattern("8B 97 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? EB ? 80 B9 ? ? ? ? ? 74", game_file)
            .count(1)
            .get(0)
            .adjust(13)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("SPAWN_SYSTEM_PARSE_TAGS", result, [&] {
        auto match = pattern("4C 8D 4C ? ? C7 44 ? ? ? ? ? ? 4C 8D ? ? ? E8 ? ? ? ? 85 C0 74", game_file)
            .count(1)
            .get(0)
            .adjust(18)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("SPAWN_SYSTEM_GET_MATCHING_RES", result, [&] {
        auto match = pattern("E8 ? ? ? ? 85 C0 74 ? 4D 8B CF", game_file)
            .count(1)
            .get(0)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("SPAWN_VEHICLE_OWNERSHIP_CALLBACK", result, [&] {
        auto match = pattern("45 33 E4 4C ? ? ? ? ? ? 44 38 A1 ? ? ? ? 74 ? 45 33 C9", game_file)
            .count(1)
            .get(0)
            .adjust(6);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("EVENT_SCHEDULER", result, [&] {
        auto match = pattern("48 8B ? ? 48 85 FF 74 ? 48 8B CF E8 ? ? ? ? 48 8B ? ? 0F B6 ? ? ?", game_file)
            .count(1)
            .get(0)
            .adjust(13);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("TELEPORT", result, [&] { // works ✅ (not needed)
        auto match = pattern("48 89 5c ? ? 48 89 6c ? ? 48 89 74 ? ? 48 89 7c ? ? ? ? 48 83 ? ? 48 8b 05 ? ? ? ? 33 db 49 8b e8", game_file)
            .count(1)
            .get(0)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    FindPattern("SET_WEATHER_PRESET_FROM_HASH", result, [&] {
        auto match = pattern("48 8B CF E8 ? ? ? ? 48 8B 0D ? ? ? ? 8B D0 E8 ? ? ? ? 84 C0 75", game_file)
            .count(1)
            .get(0)
            .adjust(17)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("RESTORE_DYNAMIC_WEATHER", result, [&] {
        auto match = pattern("48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 33 C0 EB ? 3B 1D", game_file)
            .count(1)
            .get(0)
            .adjust(7)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("WEATHER_UPDATE", result, [&] {
       auto match = pattern("85 D2 ? ? E8 ? ? ? ? 44 8B 83 ? ? ? ? 45", game_file)
           .count(1)
           .get(0)
           .adjust(4)
           .extract_call();
       return rebase(game_file, match);
    });

    FindPattern("HASHING_FUNC", result, [&] {
        auto match = pattern("49 8B ? FF ? ? ? ? ? ? ? ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 89 44 24 ? 48 8B ? ?", game_file)
            .count(1)
            .get(0)
            .adjust(23);
        return disp_rebase(game_file, match).as<uintptr_t>();
    });

    FindPattern("FUNC_VSNPRINTF", result, [&] {
        auto match = pattern("74 ? 48 8D ? ? 48 89 0E 4D 8B CE 4C 8B C5 48 8B D7 48 8B CB E8 ? ? ? ?", game_file)
            .count(1)
            .get(0)
            .adjust(21)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("FUNC_SCRIPT_ERROR_REPORT", result, [&] {
        auto match = pattern("4C 8B C3 48 8D 15 ? ? ? ? 48 8B CE E8 ? ? ? ? E9 ? ? ? ? F2 0F", game_file)
            .count(1)
            .get(0)
            .adjust(13)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("CONSUME_DEPLOYABLES", result, [&] {
        auto match = pattern("E8 ? ? ? ? 45 33 C0 0F B6 ? ? ? ? ? 8B ? ? ? ? ? E8 ? ? ? ? 48", game_file)
            .count(1)
            .get(0)
            .adjust(0)
            .extract_call();
        return rebase(game_file, match);
    });

    FindPattern("CONSUME_AMMO", result, [&] {
        auto match = pattern("85 D2 0F ? ? ? ? ? 4C 8B DC 41 56 48 83 ? ? 49", game_file)
            .count(1)
            .get(0)
            .adjust(0)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    // ++ BYTE PATCHES ++
    // (these are addresses of single bytes to patch, not functions or variables)

    // building placement patches
    FindPattern("PATCH_BUILDING_UI_CHECK_1", result, [&] { // ui collision check before building placement
        auto match = pattern("E8 ? ? ? ? 84 C0 74 04 B0 01 EB 02 32 C0 88 86", game_file).count(1).get(0).adjust(13).as<uintptr_t>();
        return rebase(game_file, match);
    });
    FindPattern("PATCH_BUILDING_UI_CHECK_2", result, [&] { // ui collision check before building placement
        auto match = pattern("48 8D 4D ? E8 ? ? ? ? 20 9E 80 ? ? ?", game_file).count(1).get(0).adjust(9).as<uintptr_t>();
        return rebase(game_file, match);
    });
    FindPattern("PATCH_BUILDING_CHECK_COLLISION", result, [&] { // collision check before building placement
        auto match = pattern("47 ? ? ? 0f 85 ? ? ? ? 45 ? ? 75 ? 45", game_file).count(1).get(0).adjust(4).as<uintptr_t>();
        return rebase(game_file, match);
    });
    FindPattern("PATCH_BUILDING_CHECK_MASTER", result, [&] { // master building check before building placement (checks collision & limit)
        auto match = pattern("48 89 5C ? ? 48 89 6C ? ? 48 89 74 ? ? 57 48 83 ? ? 80 B9 ? ? ? ? ? 48 8B D9", game_file)
            .count(1)
            .get(0)
            .adjust(0)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    // resource cheat
    FindPattern("PATCH_RESOURCE_CONSUMPTION", result, [&] { // reduces resources after building placement / crafting
        auto match = pattern("2B D7 41 ? ? ? 49 8B ?", game_file).count(1).get(0).adjust(0).as<uintptr_t>();
        return rebase(game_file, match);
    });

    // health freeze cheat
    FindPattern("PATCH_HEALTH_GOD", result, [&] { // prevents player health from changing
        auto match = pattern("40 55 56 57 41 54 41 55 41 56 41 57 48 8D AC ? ? ? ? ? 48 81 EC ? ? ? ? 48 C7 45 ? ? ? ? ? 48 89 9C ? ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B FA 48 8B F1 45 33 FF 44 89 7C ? ?", game_file)
            .count(1)
            .get(0)
            .adjust(0)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    // fuel at 100% cheat (take maximum of current fuel and max fuel)
    FindPattern("PATCH_INFINITE_FUEL", result, [&] { // 5D to 5F -> MAXSS XMM7, MaxFuel
        auto match = pattern("F3 0F 5F FE F3 0F 5D 3D ? ? ? ? F3 0F 11 BF", game_file)
            .count(1)
            .get(0)
            .adjust(4)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    // hides ALL UI (breaks menus)
    FindPattern("PATCH_HIDE_UI", result, [&] {
        auto match = pattern("F3 0F 10 ? ? 0F 29 ? ? ? F3 41 ? ? ? 44 0F", game_file)
            .count(1)
            .get(0)
            .adjust(0)
            .as<uintptr_t>();
        return rebase(game_file, match);
    });

    // makes player undetectable for machines
    FindPattern("PATCH_DETECTION", result, [&] {
        auto match = pattern("F3 41 0F 58 07 F3 41 0F 5F C0", game_file)
            .count(1)
            .get(0)
            .adjust(5)
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
                 FindPatternResult& epic_addresses)
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
        const uintptr_t    epic_addr  = epic_addresses[i].second;

        stream << "    g_Address[" << name << "] = (is_steam ? 0x" << std::hex << steam_addr << " : 0x" << std::hex << epic_addr << ") + offset;\n";
    }

    stream << "}\n\n";

    stream << "uintptr_t GetAddress(Address address)\n";
    stream << "{\n";
    stream << "    return g_Address[address];\n";
    stream << "}\n";

    stream << "}; // namespace gz";
}

int main()
{
    auto steam_addresses = Generate("Steam", R"(C:\Program Files (x86)\Steam\steamapps\common\GenerationZero\GenerationZero_F.exe)");
    auto epic_addresses  = Generate("Epic Store", R"(C:\Program Files (x86)\Steam\steamapps\common\GenerationZero\GenerationZero_F.exe)"); // TODO: change path

    assert(steam_addresses.size() == epic_addresses.size());

    char tmp[MAX_PATH] = {0};
    GetModuleFileName(nullptr, tmp, sizeof(tmp));

    std::filesystem::path path(tmp);
    std::filesystem::path base_path = path.parent_path().parent_path().parent_path().parent_path() / "src";
    std::cout << "Writing to " << base_path << "\n";
    WriteHeader(base_path / "addresses.h", steam_addresses);
    WriteSource(base_path / "addresses.cpp", steam_addresses, epic_addresses);
    return 0;
}
