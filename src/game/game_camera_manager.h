#pragma once

#include <cstdint>

#include "../addresses.h"
#include "../data_types.h"
#include "camera_manager.h"

#pragma pack(push, 1)
namespace gz
{
    struct CCameraParams
    {
        float   m_FOV;             // 0x00 → absolute 0x5F0 (next), 0x430 (prev)
        float   m_ForegroundFOV;   // 0x04 → absolute 0x5F4
        uint8_t _unknown[0x98];    // remaining DOF/blur/bloom/shadow/vignette params
    };
    static_assert(sizeof(CCameraParams) == 0xA0);

    struct CCameraContext
    {
        CMatrix4f   m_CameraTransform;      // 0x00 → 0x40
        CMatrix4f   m_ForegroundTransform;  // 0x40 → 0x80
        CMatrix4f   m_ListenerTransform;    // 0x80 → 0xC0
        CCameraParams m_Params;             // 0xC0 → 0x160
    };
    static_assert(sizeof(CCameraContext) == 0x160);

    // Inline struct inside CGameCameraManager at +0x0F0
    struct CCameraControlContext
    {
        float   m_Dt;                     // 0x000 → 0x004
        float   m_RenderDt;               // 0x004 → 0x008
        float   m_RenderDtf;              // 0x008 → 0x00C
        uint8_t _pad0[0x004];             // 0x00C → 0x010
        float   m_CAMERA_DT_EPSILON;      // 0x010 → 0x014
        uint8_t _pad1[0x2CC];             // 0x014  → 0x2E0 (unknown: entry transition,
                                          //           initial rig, weights, parent constraint etc.)

        CCameraContext m_PreviousRenderContext; // 0x2E0 → 0x440
        CCameraContext m_NextRenderContext;     // 0x440 → 0x5A0
    };
    static_assert(sizeof(CCameraControlContext) == 0x5A0);

    class CGameCameraManager
    {
    public:
        uint8_t                 _pad0[0x010];               // 0x000 → 0x010  (vtable + unknown)
        bool                    m_Active;                   // 0x010
        uint8_t                 _pad1[0x0D7];               // 0x011 → 0x0E8
        int64_t                 m_CurrentCameraStartTime;   // 0x0E8 → 0x0F0
        CCameraControlContext   m_ControlContext;           // 0x0F0 → 0x690
        uint8_t                 _pad2[0x030];               // 0x690 → 0x6C0 (unknown)
        uint8_t                 m_Tree[0x010];              // 0x6C0 → 0x6D0 CCameraTree
        uint8_t                 m_Blackboard[0x040];        // 0x6D0 → 0x710 (size uncertain)
        bool                    m_ShouldPauseGame;          // 0x710 → 0x711
        uint8_t                 _pad3[0x067];               // 0x711 → 0x778
        CCamera*                m_FirstPersonCamera;        // 0x778 → 0x780

        static CGameCameraManager* instance()
        {
            return *(CGameCameraManager**)GetAddress(INST_GAME_CAMERA_MANAGER);
        }
    };
    static_assert(offsetof(CGameCameraManager, m_FirstPersonCamera) == 0x778);
} // namespace gz
#pragma pack(pop)