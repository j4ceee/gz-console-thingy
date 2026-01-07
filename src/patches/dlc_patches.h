#pragma once
#include "patch_group.h"
#include "addresses.h"

namespace gz::DLCPatch
{
    inline const uint8_t PATCH1_ORIGINAL[] = {0x32, 0xC0}; // XOR AL,AL
    inline const uint8_t PATCH1_PATCHED[]  = {0xB0, 0x01}; // MOV AL,1

    inline const PatchInfo s_patches[] = {
        {
            PATCH_DLC_BOUNDARY,
            PATCH1_ORIGINAL,
            PATCH1_PATCHED,
            sizeof(PATCH1_ORIGINAL),
            "XOR AL,AL to MOV AL,1",
        }
    };

    inline PatchGroup& GetPatchGroup()
    {
        static PatchGroup group("DLC Boundary Patch", s_patches, sizeof(s_patches) / sizeof(PatchInfo));
        return group;
    }

    inline bool Initialize() { return GetPatchGroup().Initialize(); }
    inline bool IsInitialized() { return GetPatchGroup().IsInitialized(); }
    inline void EnableDLCBoundaryBypass() { GetPatchGroup().Enable(); }
    inline void DisableDLCBoundaryBypass() { GetPatchGroup().Disable(); }
    inline void ToggleDLCBoundaryBypass() { GetPatchGroup().Toggle(); }
    inline bool IsDLCBoundaryBypassEnabled() { return GetPatchGroup().IsEnabled(); }
}
