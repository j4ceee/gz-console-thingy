#include <Windows.h>
#include "xinput9_1_0.h"
#include "addresses.h"
#include "patches.h"
#include "log.h"
#include "MinHook.h"

static HMODULE g_original_module = nullptr;

inline bool VerifySanityCheck()
{
    auto* addr = (const char*)gz::GetAddress(gz::SANITY_CHECK);
    return !IsBadReadPtr(addr, sizeof("Avalanche Engine")) && strcmp(addr, "Avalanche Engine") == 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    // Check if we're in Generation Zero
    if (!GetModuleHandle("GenerationZero_F.exe")) {
        gz::Log("Not in Generation Zero - exiting");
        return TRUE;
    }

    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            gz::Log("DLL injected successfully into Generation Zero!");

            // load the real XInput DLL
            char buffer[MAX_PATH] = {0};
            GetSystemDirectoryA(buffer, MAX_PATH);
            strcat_s(buffer, MAX_PATH, "\\xinput9_1_0.dll");

            g_original_module = LoadLibraryA(buffer);
            if (g_original_module) {
                XInputGetState_ = (XInputGetState_t)GetProcAddress(g_original_module, "XInputGetState");
                XInputSetState_ = (XInputSetState_t)GetProcAddress(g_original_module, "XInputSetState");
                gz::Log("XInput proxy loaded successfully");
            } else {
                gz::Log("Failed to load original XInput DLL");
            }

            // do version & sanity check
            bool is_steam = true;
            gz::InitAddresses(is_steam);
            bool valid = VerifySanityCheck();

            if (!valid)
            {
                is_steam = false;
                gz::InitAddresses(is_steam);
                valid = VerifySanityCheck();
            }

            if (valid)
            {
                gz::Log("Detected version: %s", is_steam ? "Steam" : "Microsoft Store");
            }

            if (!valid || !gz::InitPatchesAndHooks())
            {
                gz::Log("Failed to initialize patches and hooks - exiting");
                MessageBox(nullptr,
                           "There was an error setting up the mod. This can happen after game updates or when using old game versions."
                           "\nMake sure you have the latest version of the mod and the game (on Steam or Microsoft Store). After game updates the mod will"
                           " have to be updated.", "Generation Zero Console Thingy", MB_OK | MB_ICONEXCLAMATION);
                return FALSE;
            }

            break;
        }

        case DLL_PROCESS_DETACH: {
            gz::Log("DLL unloading");
            MH_DisableHook(MH_ALL_HOOKS);
            MH_Uninitialize();
            if (g_original_module) {
                FreeLibrary(g_original_module);
            }
            break;
        }
    }

    return TRUE;
}