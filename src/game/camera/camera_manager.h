#pragma once

#include <cstddef>
#include <cstdint>

#include "../../addresses.h"
#include "../../data_types.h"

#pragma pack(push, 1)
namespace gz
{
    struct CFrustumPlane {
        float x, y, z, D;
    };
    static_assert(sizeof(CFrustumPlane) == 0x10);

    struct SCorner {
        int8_t x, y, z;
    };
    static_assert(sizeof(SCorner) == 3);


    class CCamera
    {
    public:
        uint8_t     _pad0[0x8];                 // 0x000

        int32_t     m_OffCenterTiles;           // 0x008 default: 1
        int32_t     m_OffCenterTileX;           // 0x00C default: 0
        int32_t     m_OffCenterTileY;           // 0x010 default: 0
        float       m_OffCenterFracX;           // 0x014 default: 0.0
        float       m_OffCenterFracY;           // 0x018 default: 0.0

        // transform matrices
        CMatrix4f   m_PreviousTransformF;       // 0x01C
        CMatrix4f   m_TransformF;               // 0x05C
        CMatrix4f   m_TransformT0;              // 0x09C
        CMatrix4f   m_TransformT1;              // 0x0DC

        CMatrix4f   _unkMatrix;                 // 0x11C unknown

        CMatrix4f   m_ProjectionF;              // 0x15C
        CMatrix4f   m_ViewProjectionF;          // 0x19C
        CMatrix4f   m_PreviousProjF;            // 0x1DC
        CMatrix4f   m_PreviousViewF;            // 0x21C
        CMatrix4f   m_PreviousViewProjectionF;  // 0x25C
        CMatrix4f   m_Projection;               // 0x29C
        CMatrix4f   m_View;                     // 0x2DC
        CMatrix4f   m_ViewProjection;           // 0x31C
        CMatrix4f   m_PreviousProj;             // 0x35C
        CMatrix4f   m_PreviousView;             // 0x39C
        CMatrix4f   m_PreviousViewProjection;   // 0x3DC

        // frustum
        CFrustumPlane   m_FrustumPlane[6];      // 0x41C default: {0, 1, 0, 0} per plane

        uint8_t     _pad1[0xD8];                // 0x47C unknown

        // closest corners
        SCorner     m_ClosestCorner[6];         // 0x554 default: {1,1,1} each

        uint8_t     m_Flags;                    // 0x566 bitfield; ctor: &= 0x98, |= 0x18
        uint8_t     _pad2[0x21];                // 0x567 unknown

        // FOV & projection scalars
        float       m_FOVT0;                    // 0x588 = m_FOV on init
        float       m_FOVT1;                    // 0x58C = m_FOV on init
        float       m_FOV;                      // 0x590 default: ~1.1345 rad (~65°)
        float       m_FOVProjFactor;            // 0x594 default: ~1.5705 (~π/2)
        float       m_FOVFactor;                // 0x598 default: 1.0 (or powf result)
        float       m_Near;                     // 0x59C default: 0.01
        float       m_Far;                      // 0x5A0 default: 40000.0
        float       m_FactorR;                  // 0x5A4 default: 0.0
        float       m_FactorU;                  // 0x5A8 default: 0.0
        float       m_AspectRatio;              // 0x5AC default: 1.0
        int32_t     m_Width;                    // 0x5B0 default: 1
        int32_t     m_Height;                   // 0x5B4 default: 1


        [[nodiscard]] int32_t GetWidth() const { return m_Width; }
        [[nodiscard]] int32_t GetHeight() const { return m_Height; }

        void SetWidth(int32_t width) { m_Width = width; }
        void SetHeight(int32_t height) { m_Height = height; }

        [[nodiscard]] float GetFOVDegrees() const { return m_FOV * (180.0f / 3.14159265f); }
        void SetFOVDegrees(float fovDegrees) { m_FOV = fovDegrees * (3.14159265f / 180.0f); }

        [[nodiscard]] float GetNearPlane() const { return m_Near; }
        void SetNearPlane(float nearPlane) { m_Near = nearPlane; }

        [[nodiscard]] float GetFarPlane() const { return m_Far; }
        void SetFarPlane(float farPlane) { m_Far = farPlane; }
    };
    static_assert(sizeof(CCamera) == 0x5B8);
    static_assert(offsetof(CCamera, m_TransformF) == 0x05C);
    static_assert(offsetof(CCamera, m_ViewProjectionF) == 0x19C);
    static_assert(offsetof(CCamera, m_FrustumPlane) == 0x41C);
    static_assert(offsetof(CCamera, m_ClosestCorner) == 0x554);
    static_assert(offsetof(CCamera, m_FOV) == 0x590);
    static_assert(offsetof(CCamera, m_Far) == 0x5A0);

    class CCameraManager
    {
    public:
        char        _pad0[0x10];        // 0x000 → 0x010 (vtable + unknown)
        CCamera     m_DefaultCamera;    // 0x010 → 0x5C8
        CCamera*    m_ActiveCamera;     // 0x5C8 → 0x5D0
        CCamera*    m_RenderCamera;     // 0x5D0 → 0x5D8
        float       m_AspectRatio;      // 0x5D8 → 0x5DC

        static CCameraManager* instance()
        {
            return *(CCameraManager**)(GetAddress(INST_CAMERA_MANAGER));
        }
    };
    static_assert(offsetof(CCameraManager, m_DefaultCamera) == 0x010);
    static_assert(offsetof(CCameraManager, m_ActiveCamera)  == 0x5C8);
    static_assert(offsetof(CCameraManager, m_RenderCamera)  == 0x5D0);
    static_assert(offsetof(CCameraManager, m_AspectRatio)   == 0x5D8);
} // namespace gz
#pragma pack(pop)