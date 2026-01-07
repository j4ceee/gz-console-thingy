#pragma once
#include "patch_group.h"
#include "addresses.h"

namespace gz::FastTravelPatches
{
    inline const uint8_t PATCH1_ORIGINAL[] = {0x45, 0x0f, 0xb6, 0x47, 0x08}; // MOVZX R8B, byte ptr [R15+08]
    inline const uint8_t PATCH1_PATCHED[]  = {0x41, 0xB0, 0x01, 0x90, 0x90}; // MOV R8B, 1; NOP; NOP

    inline const uint8_t PATCH2_ORIGINAL[] = {0x0f, 0x84, 0x60, 0x03, 0x00, 0x00}; // JZ <some address>
    inline const uint8_t PATCH2_PATCHED[]  = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}; // NOPs

    inline const PatchInfo s_patches[] = {
        {
            PATCH_MAP_FAST_TRAVEL_BUTTON,
            PATCH1_ORIGINAL,
            PATCH1_PATCHED,
            sizeof(PATCH1_ORIGINAL),
            "MOVZX R8B, byte ptr [R15+08] -> MOV R8B, 1; NOP; NOP"
        },
        {
            PATCH_MAP_FAST_TRAVEL_VALIDATION,
            PATCH2_ORIGINAL,
            PATCH2_PATCHED,
            sizeof(PATCH2_ORIGINAL),
            "JZ <addr> -> NOPs"
        }
    };

    inline PatchGroup& GetPatchGroup()
    {
        static PatchGroup group("Fast Travel", s_patches, sizeof(s_patches) / sizeof(PatchInfo));
        return group;
    }

    inline bool Initialize() { return GetPatchGroup().Initialize(); }
    inline bool IsInitialized() { return GetPatchGroup().IsInitialized(); }
    inline void EnableFastTravelAnywhere() { GetPatchGroup().Enable(); }
    inline void DisableFastTravelAnywhere() { GetPatchGroup().Disable(); }
    inline void ToggleFastTravelAnywhere() { GetPatchGroup().Toggle(); }
    inline bool IsFastTravelAnywhereEnabled() { return GetPatchGroup().IsEnabled(); }
}
