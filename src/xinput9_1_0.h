#pragma once
#include <xinput.h>
#include "game/input_manager.h"

namespace
{
using XInputGetState_t = DWORD (*)(DWORD, uintptr_t);
using XInputSetState_t = DWORD (*)(DWORD, uintptr_t);

static XInputGetState_t XInputGetState_;
static XInputSetState_t XInputSetState_;

__declspec(dllexport) DWORD XInputGetState(DWORD dwUserIndex, uintptr_t pState)
{
    if (XInputGetState_) {
        DWORD result = XInputGetState_(dwUserIndex, pState);
        if (result == ERROR_SUCCESS)
        {
            // track active input device
            const auto* state = reinterpret_cast<XINPUT_STATE*>(pState);
            const auto& gp = state->Gamepad;
            constexpr SHORT DEADZONE = 8000;
            if (gp.wButtons
                || abs(gp.sThumbLX) > DEADZONE || abs(gp.sThumbLY) > DEADZONE
                || abs(gp.sThumbRX) > DEADZONE || abs(gp.sThumbRY) > DEADZONE
                || gp.bLeftTrigger > 30 || gp.bRightTrigger > 30)
            {
                gz::NInput::g_isUsingGamepad = true;
            }
        }
        return result;
    }

    return E_FAIL;
}

__declspec(dllexport) DWORD XInputSetState(DWORD dwUserIndex, uintptr_t pVibration)
{
    if (XInputSetState_) {
        return XInputSetState_(dwUserIndex, pVibration);
    }

    return E_FAIL;
}
}; // namespace
