#pragma once
#include "addresses.h"
#include "patch_group.h"


namespace gz::UIPatches
{
    inline const uint8_t PATCH1_ORIGINAL[] = {0xF3, 0x0F, 0x10, 0x72, 0x14}; // MOVSS XMM6, [RDX+14h]
    inline const uint8_t PATCH1_PATCHED[]  = {0x0F, 0x57, 0xF6, 0x90, 0x90}; // XORPS XMM6, XMM6; NOP; NOP

    inline const PatchInfo s_patches[] = {
        {
            PATCH_HIDE_UI,
            PATCH1_ORIGINAL,
            PATCH1_PATCHED,
            sizeof(PATCH1_ORIGINAL),
            "MOVSS XMM6, [RDX+14h] -> XORPS XMM6, XMM6; NOP; NOP",
        }
    };

    inline PatchGroup& GetHUDPatchGroup()
    {
        static PatchGroup group("UI Hide", s_patches, sizeof(s_patches) / sizeof(PatchInfo));
        return group;
    }

    inline bool Initialize() { return GetHUDPatchGroup().Initialize(); }
    inline bool IsInitialized() { return GetHUDPatchGroup().IsInitialized(); }

    inline void HideHUD() { GetHUDPatchGroup().Enable(); }
    inline void UnhideHUD() { GetHUDPatchGroup().Disable(); }
    inline void ToggleHideHUD() { GetHUDPatchGroup().Toggle(); }
    inline bool IsHideHUDEnabled() { return GetHUDPatchGroup().IsEnabled(); }
}
