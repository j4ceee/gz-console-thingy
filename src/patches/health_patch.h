#pragma once
#include "addresses.h"
#include "patch_group.h"


namespace gz::HealthPatches
{
inline const uint8_t PATCH1_ORIGINAL[] = {0x40, 0x55}; // PUSH RBP
inline const uint8_t PATCH1_PATCHED[]  = {0xC3, 0x90}; // RET NOP

inline const PatchInfo s_patches[] = {
    {
        PATCH_HEALTH_GOD,
        PATCH1_ORIGINAL,
        PATCH1_PATCHED,
        sizeof(PATCH1_ORIGINAL),
        "Push RBP -> Ret NOP",
    }
};

inline PatchGroup& GetPatchGroup()
{
    static PatchGroup group("Health", s_patches, sizeof(s_patches) / sizeof(PatchInfo));
    return group;
}

inline bool Initialize() { return GetPatchGroup().Initialize(); }
inline bool IsInitialized() { return GetPatchGroup().IsInitialized(); }
inline void EnableFreezeHealth() { GetPatchGroup().Enable(); }
inline void DisableFreezeHealth() { GetPatchGroup().Disable(); }
inline void ToggleFreezeHealth() { GetPatchGroup().Toggle(); }
inline bool IsFreezeHealthEnabled() { return GetPatchGroup().IsEnabled(); }
}
