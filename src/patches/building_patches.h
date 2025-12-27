#pragma once
#include "patch_group.h"
#include "addresses.h"


namespace gz::BuildingPatches
{
    inline const uint8_t PATCH1_ORIGINAL[] = {0x32, 0xC0}; // XOR AL,AL
    inline const uint8_t PATCH1_PATCHED[]  = {0xB0, 0x01}; // MOV AL,1

    inline const uint8_t PATCH2_ORIGINAL[] = {0x20, 0x9E, 0x80, 0x03, 0x00, 0x00}; // AND byte ptr [RSI + 0x380],BL
    inline const uint8_t PATCH2_PATCHED[]  = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}; // NOPs

    inline const uint8_t PATCH3_ORIGINAL[] = {0x0F, 0x85, 0xDE, 0xFE, 0xFF, 0xFF}; // JNE LAB_140bbaef0
    inline const uint8_t PATCH3_PATCHED[]  = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}; // NOPs

    inline const uint8_t PATCH4_ORIGINAL[] = {0x48, 0x89, 0x5C, 0x24, 0x08}; // MOV qword ptr [RSP + local_res8],RBX
    inline const uint8_t PATCH4_PATCHED[]  = {0xB0, 0x01, 0xC3, 0x90, 0x90}; // MOV AL,1; ret; NOP; NOP

    inline const PatchInfo s_patches[] = {
        {
            PATCH_BUILDING_UI_CHECK_1,
            PATCH1_ORIGINAL,
            PATCH1_PATCHED,
            sizeof(PATCH1_ORIGINAL),
            "XOR AL,AL -> MOV AL,1"
        },
        {
            PATCH_BUILDING_UI_CHECK_2,
            PATCH2_ORIGINAL,
            PATCH2_PATCHED,
            sizeof(PATCH2_ORIGINAL),
            "AND [RSI+380],BL -> NOPs"
        },
        {
            PATCH_BUILDING_CHECK_COLLISION,
            PATCH3_ORIGINAL,
            PATCH3_PATCHED,
            sizeof(PATCH3_ORIGINAL),
            "JNE LAB_140bbaef0 -> NOPs"
        },
        {
            PATCH_BUILDING_CHECK_MASTER,
            PATCH4_ORIGINAL,
            PATCH4_PATCHED,
            sizeof(PATCH4_ORIGINAL),
            "MOV [RSP+8],RBX -> MOV AL,1; RET"
        }
    };

    inline PatchGroup& GetPatchGroup()
    {
        static PatchGroup group("Building", s_patches, sizeof(s_patches) / sizeof(PatchInfo));
        return group;
    }

    inline bool Initialize() { return GetPatchGroup().Initialize(); }
    inline bool IsInitialized() { return GetPatchGroup().IsInitialized(); }
    inline void EnableFreeBuild() { GetPatchGroup().Enable(); }
    inline void DisableFreeBuild() { GetPatchGroup().Disable(); }
    inline void ToggleFreeBuild() { GetPatchGroup().Toggle(); }
    inline bool IsFreeBuildEnabled() { return GetPatchGroup().IsEnabled(); }
}
