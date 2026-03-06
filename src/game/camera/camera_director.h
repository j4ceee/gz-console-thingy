#pragma once

#include <cstdint>

#include "addresses.h"
#include "data_types.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
    struct SCameraId
    {
        uint16_t m_Hash[3];    // 6 bytes
        uint16_t m_UserData;   // 2 bytes
    };
    static_assert(sizeof(SCameraId) == 8);

    struct SContext
    {
        void*    m_Callback;   // 0x00
        uint64_t _pad;         // 0x08 unknown
        void*    m_UserData;   // 0x10
    };
    static_assert(sizeof(SContext) == 24);

    struct KnownCameraIds
    {
        // stable hashes, confirmed across restarts
        static constexpr uint16_t HASH_ON_FOOT[3]    = { 0xE3DA, 0x4DC3, 0x35BC };
        static constexpr uint16_t HASH_SPRINTING[3]  = { 0x3176, 0xD192, 0x6913 };
        static constexpr uint16_t HASH_CROUCHING[3]  = { 0x7244, 0x57BA, 0xD699 };
        static constexpr uint16_t HASH_VEHICLE[3]    = { 0xBB04, 0xC7E1, 0xA714 };
        static constexpr uint16_t HASH_REMOTE[3]     = { 0xC3CE, 0x65B6, 0x374F };
        static constexpr uint16_t HASH_EMOTE[3]      = { 0x5E5A, 0x1DBA, 0xC16F };

        SCameraId onFoot   = {};
        SCameraId vehicle  = {};
        SCameraId remote   = {};
        SCameraId emote    = {};

        bool onFootCached  = false;
        bool vehicleCached = false;
        bool remoteCached  = false;
        bool emoteCached   = false;

        static bool HashMatches(const SCameraId& id, const uint16_t hash[3])
        {
            return id.m_Hash[0] == hash[0]
                && id.m_Hash[1] == hash[1]
                && id.m_Hash[2] == hash[2];
        }

        void TryCache(const SCameraId& id)
        {
            if (!onFootCached  && HashMatches(id, HASH_ON_FOOT))  { onFoot  = id; onFootCached  = true; }
            if (!vehicleCached && HashMatches(id, HASH_VEHICLE))  { vehicle = id; vehicleCached = true; }
            if (!remoteCached  && HashMatches(id, HASH_REMOTE))   { remote  = id; remoteCached  = true; }
            if (!emoteCached   && HashMatches(id, HASH_EMOTE))    { emote   = id; emoteCached   = true; }
        }

        bool AllCached() const
        {
            return onFootCached && vehicleCached && remoteCached && emoteCached;
        }

        /// <summary>
        /// Checks if the given camera ID matches any of the known standard camera types (on-foot, sprinting, crouching).
        /// </summary>
        static bool IsStandardCamera(const SCameraId& id)
        {
            return (
                HashMatches(id, HASH_ON_FOOT)
                || HashMatches(id, HASH_SPRINTING)
                || HashMatches(id, HASH_CROUCHING)
            );
        }
    };

    inline KnownCameraIds g_cameraIds;


    using PushCameraFunc = void(*)(void* camDir, SCameraId* camId);
    inline PushCameraFunc g_pushCamera = nullptr;

    inline bool g_forceEmoteCamera = false;
    inline bool g_forceVehicleCamera = false;
    inline bool g_forceRemoteCamera = false;

    class CCameraDirector
    {
    public:
        uint8_t     _pad0[0x10];                // 0x000 → 0x010  (vtable + unknown)

        SContext    m_Contexts[16];             // 0x010 → 0x190

        int32_t     m_ContextCount;             // 0x190 → 0x194
        SCameraId   m_DefaultCamera;            // 0x194 → 0x19C
        SCameraId   m_StackTopCamera;           // 0x19C → 0x1A4
        uint8_t     _pad1[0x14];                // 0x1A4 → 0x1B8  (unknown)

        BasicVector m_RequestedCameras;         // 0x1B8 → 0x1D0 (triplet of SCameraId*)
        BasicVector m_CameraStack;              // 0x1D0 → 0x1E8 (triplet of SCameraId*)

        SCameraId   m_SelectedCamera;           // 0x1E8 → 0x1F0
        void*       m_SelectedTransition;       // 0x1F0 → 0x1F8

        static CCameraDirector* instance()
        {
            return *(CCameraDirector**)(GetAddress(INST_CAMERA_DIRECTOR));
        }

        void PushCamera(SCameraId* cameraId)
        {
            meow_hook::func_call<int>(
                GetAddress(DIRECTOR_PUSH_CAMERA),
                this,
                cameraId
            );
        }

        void ResetToDefaultCamera()
        {
            this->PushCamera(&m_DefaultCamera);
        }

        static void Hooked_PushCamera(void* camDir, SCameraId* camId)
        {
            if (camId)
            {
                g_cameraIds.TryCache(*camId);

                if (g_forceEmoteCamera && KnownCameraIds::IsStandardCamera(*camId))
                {
                    camId = &g_cameraIds.emote; // redirect to emote camera when trying to switch to on-foot camera
                }
                else if (g_forceVehicleCamera && KnownCameraIds::IsStandardCamera(*camId))
                {
                    camId = &g_cameraIds.vehicle; // redirect to vehicle camera when trying to switch to on-foot camera
                }
                else if (g_forceRemoteCamera && KnownCameraIds::IsStandardCamera(*camId))
                {
                    camId = &g_cameraIds.remote; // redirect to remote camera when trying to switch to on-foot camera
                }
            }

            // call original function
            if (g_pushCamera)
            {
                g_pushCamera(camDir, camId);
            }
        }

        static bool SetupPushCameraHook()
        {
            if (g_pushCamera != nullptr) {
                return true; // already set up
            }
            return MH_CreateHookGZ(
                DIRECTOR_PUSH_CAMERA,
                &Hooked_PushCamera,
                &g_pushCamera
            );
        }
    };
} // namespace gz
#pragma pack(pop)