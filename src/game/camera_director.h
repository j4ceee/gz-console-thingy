#pragma once

#include <cstdint>
#include "../addresses.h"
#include "../data_types.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
    struct SCameraId
    {
        uint16_t m_Hash[3];    // 6 bytes
        uint16_t m_UserData;   // 2 bytes

        bool operator==(const SCameraId& camera_id) const
        {
            return this->m_Hash[0] == camera_id.m_Hash[0]
                && this->m_Hash[1] == camera_id.m_Hash[1]
                && this->m_Hash[2] == camera_id.m_Hash[2]
                && this->m_UserData == camera_id.m_UserData;
        }
    };
    static_assert(sizeof(SCameraId) == 8);

    struct SContext
    {
        void*    m_Callback;   // 0x00
        uint64_t _pad;         // 0x08 unknown
        void*    m_UserData;   // 0x10
    };
    static_assert(sizeof(SContext) == 24);

    class CCameraDirector
    {
    public:
        uint8_t     _pad0[0x10];                // 0x000 → 0x010  (vtable + unknown)
        SContext    m_Contexts[16];             // 0x010 → 0x190
        int32_t     m_ContextCount;             // 0x190 → 0x194
        SCameraId   m_DefaultCamera;            // 0x194 → 0x19C
        SCameraId   m_StackTopCamera;           // 0x19C → 0x1A4
        uint8_t     _pad1[0x14];                // 0x1A4 → 0x1B8 (unknown)
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

        void PopCamera(SCameraId* cameraId)
        {
            meow_hook::func_call<void>(
                GetAddress(DIRECTOR_POP_CAMERA),
                this,
                cameraId
            );
        }

        void ResetToDefaultCamera()
        {
            this->PushCamera(&m_DefaultCamera);
        }
    };
} // namespace gz
#pragma pack(pop)