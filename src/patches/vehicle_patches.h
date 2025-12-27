#pragma once
#include "patch_group.h"


namespace gz::VehiclePatches
{
    inline const uint8_t PATCH1_ORIGINAL[] = {0xF3, 0x0F, 0x5D}; // MINSS
    inline const uint8_t PATCH1_PATCHED[]  = {0xF3, 0x0F, 0x5F}; // MAXSS

    inline const PatchInfo s_patches[] = {
        {
            PATCH_INFINITE_FUEL,
            PATCH1_ORIGINAL,
            PATCH1_PATCHED,
            sizeof(PATCH1_ORIGINAL),
            "MINSS -> MAXSS"
        }
    };

    inline PatchGroup& GetPatchGroup()
    {
        static PatchGroup group("Vehicle", s_patches, sizeof(s_patches) / sizeof(PatchInfo));
        return group;
    }

    inline bool Initialize() { return GetPatchGroup().Initialize(); }
    inline bool IsInitialized() { return GetPatchGroup().IsInitialized(); }
    inline void EnableInfiniteFuel() { GetPatchGroup().Enable(); }
    inline void DisableInfiniteFuel() { GetPatchGroup().Disable(); }
    inline void ToggleInfiniteFuel() { GetPatchGroup().Toggle(); }
    inline bool IsInfiniteFuelEnabled() { return GetPatchGroup().IsEnabled(); }
}
