#include <Windows.h>
#include <filesystem>

#include "xinput9_1_0.h"

#include "input.h"

#include "addresses.h"
#include "patches.h"
#include "log.h"

static HMODULE g_original_module = nullptr;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    // Check if we're in Generation Zero
    if (!GetModuleHandle("GenerationZero_F.exe")) {
        gz::Log("Not in Generation Zero - exiting");
        return FALSE;
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

            bool is_steam = !strstr(GetCommandLine(), "-epicusername") && GetModuleHandle("steam_api64.dll");
            gz::Log("Is Steam version: %s", is_steam ? "true" : "false");
            gz::InitAddresses(is_steam);
            if (!gz::InitPatchesAndHooks()) {
                gz::Log("Failed to initialize patches and hooks - exiting");
                return FALSE;
            }

            break;
        }

        case DLL_PROCESS_DETACH: {
            gz::Log("DLL unloading");
            if (g_original_module) {
                FreeLibrary(g_original_module);
            }
            break;
        }
    }

    return TRUE;
}