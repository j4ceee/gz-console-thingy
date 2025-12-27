#pragma once
#include "patch_group.h"


namespace gz::ResourcePatches
{
    inline const uint8_t PATCH1_ORIGINAL[] = {0x2B, 0xD7}; // SUB EDX,EDI
    inline const uint8_t PATCH1_PATCHED[]  = {0x8B, 0xD2}; // MOV EDX,EDX

    inline const PatchInfo s_patches[] = {
        {
            PATCH_RESOURCE_CONSUMPTION,
            PATCH1_ORIGINAL,
            PATCH1_PATCHED,
            sizeof(PATCH1_ORIGINAL),
            "XOR AL,AL -> MOV AL,1"
        }
    };

    inline PatchGroup& GetPatchGroup()
    {
        static PatchGroup group("Resources", s_patches, sizeof(s_patches) / sizeof(PatchInfo));
        return group;
    }

    inline bool Initialize() { return GetPatchGroup().Initialize(); }
    inline bool IsInitialized() { return GetPatchGroup().IsInitialized(); }
    inline void EnableUnlimitedResources() { GetPatchGroup().Enable(); }
    inline void DisableUnlimitedResources() { GetPatchGroup().Disable(); }
    inline void ToggleUnlimitedResources() { GetPatchGroup().Toggle(); }
    inline bool IsUnlimitedResourcesEnabled() { return GetPatchGroup().IsEnabled(); }
}
