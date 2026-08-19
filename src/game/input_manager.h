#pragma once

#include "addresses.h"
#include "imgui/ui.h"
#include <cstdint>

namespace gz::NInput
{
    inline bool g_isUsingGamepad = false;

    using FocusFunc_t = void(*)(void*);
    inline FocusFunc_t g_origGotFocus  = nullptr;
    inline FocusFunc_t g_origLostFocus = nullptr;

    #pragma pack(push, 1)
    class CManager
    {
    public:
        void* m_impl;

        static CManager* instance()
        {
            return *(CManager**)GetAddress(INST_INPUT_MANAGER);
        }

        void LoseFocus() const
        {
            if (g_origLostFocus) g_origLostFocus(m_impl);
        }

        void GainFocus() const
        {
            if (g_origGotFocus) g_origGotFocus(m_impl);
        }
    };

    struct CGamepadImpl
    {
        uint8_t _pad0[0x540];
        uint8_t stateBuffer[0x70]; // current-frame axis/button values
        uint32_t lastInputTimestamp; // 0x5b0
        bool isConnected; // 0x5b4
        bool wasConnected; // 0x5b5
    };
    static_assert(offsetof(CGamepadImpl, stateBuffer) == 0x540);
    static_assert(offsetof(CGamepadImpl, isConnected) == 0x5b4);

    struct CGamepad
    {
        CGamepadImpl* m_Impl; // head of unique_ptr, effectively raw ptr
    };
    #pragma pack(pop)


    inline void HookedGotFocus(void* impl)
    {
        if (UI::Get()->IsVisible()) return; // suppress game-side regain while mod UI is open
        g_origGotFocus(impl);
    }

    inline void HookedLostFocus(void* impl)
    {
        if (UI::Get()->IsVisible()) return; // suppress game-side loss while mod UI is open
        g_origLostFocus(impl);
    }

    inline bool SetupFocusHooks()
    {
        bool ok = true;
        ok &= MH_CreateHookGZ(INPUT_GOT_FOCUS,  &HookedGotFocus,  &g_origGotFocus);
        ok &= MH_CreateHookGZ(INPUT_LOST_FOCUS, &HookedLostFocus, &g_origLostFocus);
        return ok;
    }

    // -- gamepad handling --

    using updateGamepadFunc = void(*)(CGamepad*, uint32_t, bool);
    inline updateGamepadFunc g_updateGamepad = nullptr;

    inline void HookedUpdateGamepad(CGamepad* thisptr, uint32_t timestamp, bool force)
    {
        if (UI::Get()->IsVisible()) {
            memset(thisptr->m_Impl->stateBuffer, 0, sizeof(CGamepadImpl::stateBuffer));
            return;
        }
        g_updateGamepad(thisptr, timestamp, force);
    }

    inline bool SetupUpdateGamepad()
    {
        if (g_updateGamepad != nullptr) {
            return true; // already set up
        }
        return MH_CreateHookGZ(UPDATE_GAMEPAD, &HookedUpdateGamepad, &g_updateGamepad);
    }
} // namespace gz::NInput
