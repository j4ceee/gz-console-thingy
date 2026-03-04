#pragma once

#include "addresses.h"
#include "hook_helpers.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
    class CRemoteController;
    class CCharacter;

    using GetSignalStrengthFunc = float(*)(CRemoteController*, void*, void*, void*);
    inline GetSignalStrengthFunc g_getSignalStrength = nullptr;

    inline bool g_unlimitedSignalStrength = true;

    class CRemoteController
    {
    public:
        char        _pad[0xD8];             // 0x00 → 0xD8
        CCharacter* m_controlledCharacter;  // +0xD8 - the tick/machine being controlled
        void*       m_controlledEntityRef;  // +0xE0 - reference wrapper
        void*       m_inputContext;         // +0x118 - NInput::CContext for "ACT_USE_TICK"
        bool        m_someFlag;             // +0x128 - related to cleanup
        bool        m_anotherFlag;          // +0x129 - related to timing

        CCharacter* GetControlledCharacter() const
        {
            return m_controlledCharacter;
        }

        void RequestControlOfCharacter(CCharacter* character)
        {
            meow_hook::func_call<int>(
                GetAddress(REQUEST_ANIMAL_CONTROL),
                this,       // self
                character,  // character to control
                (uint64_t)1 // unknown
            );
        }

        void ReleaseControl()
        {
            meow_hook::func_call<void>(
                GetAddress(RELEASE_ANIMAL_CONTROL),
                this // self
            );
        }


        static float HookedGetSignalStrength(CRemoteController* controller, void* p2, void* p3, void* p4)
        {
            if (g_unlimitedSignalStrength) {
                return 1.0f;  // always full signal (100%)
            }

            if (g_getSignalStrength) {
                return g_getSignalStrength(controller, p2, p3, p4);
            }
            return 1.0f;  // fallback
        }

        static bool SetupSignalStrengthHook()
        {
            if (g_getSignalStrength != nullptr) {
                return true;
            }
            return MH_CreateHookGZ(GET_SIGNAL_STRENGTH, &HookedGetSignalStrength, &g_getSignalStrength);
        }
    };
} // namespace gz
#pragma pack(pop)