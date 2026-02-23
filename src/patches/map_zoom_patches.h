#pragma once
#include "addresses.h"
#include "patch_group.h"

namespace gz::MapZoomPatch
{
    inline const uint8_t PATCH_ORIGINAL[] = {0xf3, 0x0f, 0x5d, 0x1d, 0x76, 0x3d, 0xf2, 0x00};
    inline const uint8_t PATCH_PATCHED[]  = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};

    inline const PatchInfo s_patches[] = {
        {
            PATCH_MAP_ZOOM,
            PATCH_ORIGINAL,
            PATCH_PATCHED,
            sizeof(PATCH_ORIGINAL),
            "MINSS XMM3, [24.0] -> NOPs (removes max zoom cap)"
        }
    };

    inline PatchGroup& GetPatchGroup()
    {
        static PatchGroup group("Disable Map Zoom Limit", s_patches, sizeof(s_patches) / sizeof(PatchInfo));
        return group;
    }

    inline bool Initialize() { return GetPatchGroup().Initialize(); }
    inline bool IsInitialized() { return GetPatchGroup().IsInitialized(); }

    inline void DisableMapZoomLimit() { GetPatchGroup().Enable(); }
    inline void EnableMapZoomLimit() { GetPatchGroup().Disable(); }
    inline void ToggleMapZoomLimit() { GetPatchGroup().Toggle(); }
    inline bool IsMapZoomLimitEnabled() { return GetPatchGroup().IsEnabled(); }
}
