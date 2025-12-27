#pragma once
#include "addresses.h"
#include "patch_group.h"


namespace gz::CloudPatch
{
    inline const uint8_t PATCH1_ORIGINAL[] = {0x81, 0x80, 0x80, 0x3C}; // 0.015686275f
    inline const uint8_t PATCH1_PATCHED[] = {0x00, 0x00, 0x80, 0x3F}; // 1.0f

    inline const PatchInfo s_patches[] = {
        {
            VAR_CLOUD_VISIBILITY,
            PATCH1_ORIGINAL,
            PATCH1_PATCHED,
            sizeof(PATCH1_ORIGINAL),
            "0.015686275f -> 1.0f",
        }
    };

    inline PatchGroup& GetPatchGroup()
    {
        static PatchGroup group("Clouds Hide", s_patches, sizeof(s_patches) / sizeof(PatchInfo));
        return group;
    }

    inline bool Initialize() { return GetPatchGroup().Initialize(); }
    inline bool IsInitialized() { return GetPatchGroup().IsInitialized(); }

    inline void HideClouds() { GetPatchGroup().Enable(); }
    inline void UnhideClouds() { GetPatchGroup().Disable(); }
    inline void ToggleHideClouds() { GetPatchGroup().Toggle(); }
    inline bool IsHideCloudsEnabled() { return GetPatchGroup().IsEnabled(); }
}
