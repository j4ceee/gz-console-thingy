#pragma once
#include "addresses.h"
#include "patch_group.h"


namespace gz::DetectionPatch
{
    inline const uint8_t PATCH1_ORIGINAL[] = {0xF3, 0x41, 0x0F, 0x5F, 0xC0}; // MAXSS XMM0, XMM8
    inline const uint8_t PATCH1_PATCHED[]  = {0xF3, 0x41, 0x0F, 0x10, 0xC0}; // MOVSS XMM0, XMM8

    inline const PatchInfo s_patches[] = {
        {
            PATCH_DETECTION,
            PATCH1_ORIGINAL,
            PATCH1_PATCHED,
            sizeof(PATCH1_ORIGINAL),
            "MOVSS XMM0, XMM8 -> MAXSS XMM0, XMM8",
        }
    };

    inline PatchGroup& GetPatchGroup()
    {
        static PatchGroup group("UI Hide", s_patches, sizeof(s_patches) / sizeof(PatchInfo));
        return group;
    }

    inline bool Initialize() { return GetPatchGroup().Initialize(); }
    inline bool IsInitialized() { return GetPatchGroup().IsInitialized(); }

    inline void DisableDetection() { GetPatchGroup().Enable(); }
    inline void EnableDetection() { GetPatchGroup().Disable(); }
    inline void ToggleDetection() { GetPatchGroup().Toggle(); }
    inline bool IsDetectionDisabled() { return GetPatchGroup().IsEnabled(); }
}
