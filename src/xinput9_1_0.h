#pragma once
#include <xinput.h>
#include "game/input_manager.h"
#include "game/custom/free_cam_state.h"

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
            auto* state = reinterpret_cast<XINPUT_STATE*>(pState);
            const auto& gp = state->Gamepad;
            gz::NInput::g_pad = {
                gp.sThumbLX / 32767.0f, gp.sThumbLY / 32767.0f,
                gp.sThumbRX / 32767.0f, gp.sThumbRY / 32767.0f,
                gp.bLeftTrigger / 255.0f, gp.bRightTrigger / 255.0f, gp.wButtons
            };
            constexpr SHORT DEADZONE = 8000;
            if (gp.wButtons
                || abs(gp.sThumbLX) > DEADZONE || abs(gp.sThumbLY) > DEADZONE
                || abs(gp.sThumbRX) > DEADZONE || abs(gp.sThumbRY) > DEADZONE
                || gp.bLeftTrigger > 30 || gp.bRightTrigger > 30)
            {
                gz::NInput::g_isUsingGamepad = true;
            }

            // free cam owns the pad; game polls through here, so it gets an idle pad while g_pad keeps the real state
            if (gz::FreeCamState::g_enabled)
                state->Gamepad = {};
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
