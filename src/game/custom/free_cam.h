#pragma once
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <Windows.h>
#include <xinput.h>

#include "addresses.h"
#include "log.h"
#include "hook_helpers.h"
#include "imgui/ui.h"
#include "game/character.h"
#include "game/game_camera_manager.h"
#include "game/game_state.h"
#include "game/game_world.h"
#include "game/input_manager.h"
#include "game/custom/free_cam_state.h"
#include "game/custom/tp_state.h"

#pragma pack(push, 1)
namespace gz::FreeCamera
{
    using UpdateRenderContextsFunc = void(*)(void* tree, CCameraControlContext* ctx);
    inline UpdateRenderContextsFunc g_updateRenderContexts = nullptr;

    // settings
    inline float g_speed = 10.0f; // m/s
    inline float g_sensitivity = 1.0f;

    inline constexpr float kMouseRadPerCount = 0.0022f;
    inline constexpr float kPadRadPerSec = 2.2f;
    inline constexpr float kPadDeadzone = 0.15f;
    inline constexpr float kBoostMultiplier = 4.0f;
    inline constexpr float kMaxPitch = 1.553f; // ~89 deg

    // camera state (render thread)
    inline float g_yaw = 0.0f;
    inline float g_pitch = 0.0f;
    inline CVector3f g_pos{};

    inline CMatrix4f g_cameraTransform{}; // last pose written by Apply, read on the game tick

    // input state: filled by Input::FeedEvent (window thread), consumed by the camera hook
    inline bool g_keys[256]{};
    inline std::atomic<long> g_mouseDx{0};
    inline std::atomic<long> g_mouseDy{0};
    inline std::atomic<long> g_wheelDelta{0};

    inline void ResetInput()
    {
        memset(g_keys, 0, sizeof(g_keys));
        g_mouseDx = 0;
        g_mouseDy = 0;
        g_wheelDelta = 0;
    }

    /// forward = -row2, world is Y up. yaw grows to the LEFT
    inline void Seed(const CMatrix4f& m) // start exactly where the game camera is
    {
        const CVector3f f{-m.m[2].x, -m.m[2].y, -m.m[2].z};
        g_yaw = atan2f(f.x, f.z);
        g_pitch = asinf(std::clamp(f.y, -1.0f, 1.0f));
        g_pos = {m.m[3].x, m.m[3].y, m.m[3].z};
    }

    inline void Build(CMatrix4f& m) // orthonormal, no roll, no drift
    {
        const float cp = cosf(g_pitch);
        const CVector3f back{-sinf(g_yaw) * cp, -sinf(g_pitch), -cosf(g_yaw) * cp};
        const CVector3f right{-cosf(g_yaw), 0.f, sinf(g_yaw)}; // cross(worldUp, back), already unit
        const CVector3f up{
            back.y * right.z - back.z * right.y,
            back.z * right.x - back.x * right.z,
            back.x * right.y - back.y * right.x
        }; // cross(back, right)
        m.m[0] = {right.x, right.y, right.z, 0.f};
        m.m[1] = {up.x, up.y, up.z, 0.f};
        m.m[2] = {back.x, back.y, back.z, 0.f};
        m.m[3] = {g_pos.x, g_pos.y, g_pos.z, 1.f};
    }

    inline float ApplyDeadzone(float v)
    {
        const float a = std::fabs(v);
        return a < kPadDeadzone ? 0.0f : std::copysign((a - kPadDeadzone) / (1.0f - kPadDeadzone), v);
    }

    inline void Integrate(float dt)
    {
        dt = std::clamp(dt, 0.0f, 0.1f);

        // UI owns input while it's open. its key-ups never reach us, so drop stale state
        if (UI::Get()->IsVisible())
        {
            ResetInput();
            return;
        }

        float forward = 0.0f, right = 0.0f, up = 0.0f, boost = 1.0f;
        float yawDelta = 0.0f, pitchDelta = 0.0f;

        // keyboard + mouse
        forward += (g_keys['W'] ? 1.0f : 0.0f) - (g_keys['S'] ? 1.0f : 0.0f);
        right += (g_keys['D'] ? 1.0f : 0.0f) - (g_keys['A'] ? 1.0f : 0.0f);
        up += ((g_keys['E'] || g_keys[VK_SPACE]) ? 1.0f : 0.0f) - ((g_keys['Q'] || g_keys[VK_CONTROL]) ? 1.0f : 0.0f);
        if (g_keys[VK_SHIFT]) boost = kBoostMultiplier;

        yawDelta -= static_cast<float>(g_mouseDx.exchange(0)) * kMouseRadPerCount * g_sensitivity;
        pitchDelta -= static_cast<float>(g_mouseDy.exchange(0)) * kMouseRadPerCount * g_sensitivity;

        if (const long wheel = g_wheelDelta.exchange(0); wheel != 0) // wheel scales the speed
            g_speed = std::clamp(g_speed * std::pow(1.15f, static_cast<float>(wheel) / WHEEL_DELTA), 0.5f, 500.0f);

        // gamepad (real state recorded by the xinput proxy, the game's own copy is muted)
        if (NInput::g_isUsingGamepad)
        {
            const auto& pad = NInput::g_pad;
            forward += ApplyDeadzone(pad.ly);
            right += ApplyDeadzone(pad.lx);
            up += pad.rt - pad.lt;
            yawDelta -= ApplyDeadzone(pad.rx) * kPadRadPerSec * g_sensitivity * dt;
            pitchDelta += ApplyDeadzone(pad.ry) * kPadRadPerSec * g_sensitivity * dt;
            if (pad.buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) boost = kBoostMultiplier;
        }

        g_yaw += yawDelta;
        g_pitch = std::clamp(g_pitch + pitchDelta, -kMaxPitch, kMaxPitch);

        const float len = std::sqrt(forward * forward + right * right + up * up);
        if (len > 1.0f)
        {
            forward /= len;
            right /= len;
            up /= len;
        } // no faster diagonals

        const float cp = cosf(g_pitch);
        const CVector3f f{sinf(g_yaw) * cp, sinf(g_pitch), cosf(g_yaw) * cp};
        const CVector3f r{-cosf(g_yaw), 0.0f, sinf(g_yaw)};
        const float step = g_speed * boost * dt;
        g_pos += f * (forward * step) + r * (right * step) + CVector3f{0.0f, 1.0f, 0.0f} * (up * step);
    }

    inline void Apply(CCameraControlContext* ctx)
    {
        auto& next = ctx->m_NextRenderContext;

        // left the game (main menu / loading): never override a menu camera
        if (!GameState::IsInGame())
        {
            FreeCamState::g_enabled = false;
            FreeCamState::g_parked = false; // character is gone with the world
        }

        const bool want = FreeCamState::g_enabled;

        // enable free cam
        if (want && !FreeCamState::g_active)
        {
            Seed(next.m_CameraTransform);
            ResetInput();
            FreeCamState::g_active = true;
        }
        // disable free cam
        else if (!want && FreeCamState::g_active)
        {
            // game's own result is already in `next`, just stop overriding
            FreeCamState::g_active = false;
            ResetInput();
            ctx->m_PreviousRenderContext.m_ListenerTransform = next.m_ListenerTransform; // no doppler spike
            return;
        }
        if (!FreeCamState::g_active) return;

        Integrate(ctx->m_RenderDt);
        Build(next.m_CameraTransform);
        g_cameraTransform = next.m_CameraTransform;
        next.m_ListenerTransform = next.m_CameraTransform; // audio follows the camera
        // m_ForegroundTransform intentionally untouched
    }

    /// called from Input::FeedEvent. returns true if the message was consumed
    inline bool HandleInput(uint32_t message, WPARAM wParam, LPARAM lParam)
    {
        if (!FreeCamState::g_enabled) return false;

        switch (message)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (wParam < 256) g_keys[wParam] = true; // record in g_keys, swallow
            return true;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (wParam < 256) g_keys[wParam] = false; // record
            return false; // let releases through so keys held before enabling can't stick in the game
        case WM_CHAR:
            return true; // ignore text input
        case WM_INPUT: // raw mouse movement
            {
                RAWINPUT raw; // receives event
                UINT size = sizeof(raw);
                if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER))
                    != static_cast<UINT>(-1) // verify reading succeeded
                    && raw.header.dwType == RIM_TYPEMOUSE && !(raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) // ensure this is relative mouse input
                {
                    g_mouseDx += raw.data.mouse.lLastX;
                    g_mouseDy += raw.data.mouse.lLastY;
                }
                return true;
            }
        case WM_MOUSEWHEEL:
            g_wheelDelta += GET_WHEEL_DELTA_WPARAM(wParam); // scales fly speed, swallow
            return true;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONDBLCLK:
        case WM_MOUSEMOVE:
            return true; // swallow -> no shooting or aiming & no cursor movement reaching the game
        default:
            return false;
        }
    }

    inline void OnGameUpdate(CCharacter* player)
    {
        if (FreeCamState::g_active && !FreeCamState::g_parked)
        {
            player->ForceNeutralState();
            player->SetGhostMode(true);
            player->SetVisible(false); // fp arms, weapon, body
            player->Enable(false); // no physics, no ai target, no update
            FreeCamState::g_parked = true;
        }

        if (FreeCamState::g_parked && FreeCamState::g_active)
        {
            player->SetTransform(&g_cameraTransform); // parked player follows the camera
        }
        else if (FreeCamState::g_parked) // free cam was switched off
        {
            FreeCamState::g_parked = false;

            player->Enable(true); // no-op if already enabled, guarantees the registrations are back
            player->SetGhostMode(false);
            if (auto* world = CGameWorld::instance())
                world->TeleportPlayer(&g_cameraTransform);
        }
    }

    inline void HookedUpdateRenderContexts(void* tree, CCameraControlContext* ctx)
    {
        g_updateRenderContexts(tree, ctx); // game fills Next (after copying Next -> Previous)
        Apply(ctx);
    }

    inline bool SetupUpdateRenderContextsHook()
    {
        if (g_updateRenderContexts != nullptr)
        {
            return true; // already set up
        }
        return MH_CreateHookGZ(CAMERA_TREE_UPDATE_RENDER_CONTEXTS, &HookedUpdateRenderContexts, &g_updateRenderContexts);
    }

    inline bool IsHookInitialized() { return g_updateRenderContexts != nullptr; }

    /// why free cam can't be started right now, nullptr if it can
    inline const char* GetBlockReason()
    {
        auto* mgr = CNetworkPlayerManager::instance();
        auto* character = mgr ? mgr->GetCharacter() : nullptr;
        if (!character) return "No player character.";
        if (const char* why = character->GetControlBlockReason()) return why;
        if (TpState::IsActive()) return "Not available in third person.";
        if (auto* ctrl = character->GetAnimatedModel()->GetAnimationControl(); ctrl && ctrl->IsPlayingCustomAnimation())
            return "Not available while emoting.";
        return nullptr;
    }

    inline void Enable(bool v)
    {
        if (v)
        {
            if (!IsHookInitialized() || !GameState::IsInGame()) return;
            if (const char* why = GetBlockReason())
            {
                Log("Free cam: %s", why);
                return;
            }
        }
        FreeCamState::g_enabled = v;
    }

    inline void Toggle() { Enable(!FreeCamState::g_enabled); }
}
#pragma pack(pop)
