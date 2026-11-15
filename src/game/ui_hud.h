#pragma once

#include "addresses.h"
#include "hook_helpers.h"
#include "player.h"
#include "custom/free_cam_state.h"

#pragma pack(push, 1)
namespace gz
{
    class CHUDUI;

    using HudUpdateStateFunc = void(*)(CHUDUI*, CPlayer*);
    inline HudUpdateStateFunc g_hudUpdateState = nullptr;

    class CHUDUI
    {
    public:
        static CHUDUI* instance()
        {
            return *(CHUDUI**)GetAddress(INST_HUD_UI);
        }

        static void HookedUpdateState(CHUDUI* hud, CPlayer* player)
        {
            // CHUDUI::UpdateState forces HUD_HIDE_ALL while the player character's "enabled" bit is clear (parked by the free cam).<br>
            // HUD_HIDE_ALL freezes the HUD (CHUDUI::Update skips its per-frame refresh)

            constexpr uint8_t kEnabledBit = 0x08; // bit 3 of CCharacter::m_visibleFlags, written by CCharacter::Enable

            CCharacter* character = (FreeCamState::g_parked && player) ? player->GetCharacter() : nullptr;
            const bool fakeEnabled = character && !(character->m_visibleFlags & kEnabledBit);

            if (fakeEnabled && character) character->m_visibleFlags |= kEnabledBit;
            g_hudUpdateState(hud, player);
            if (fakeEnabled && character) character->m_visibleFlags &= static_cast<uint8_t>(~kEnabledBit);
        }

        static bool SetupUpdateStateHook()
        {
            if (g_hudUpdateState != nullptr) return true;
            return MH_CreateHookGZ(FUNC_HUD_UPDATE_STATE, &HookedUpdateState, &g_hudUpdateState);
        }
    };
} // namespace gz
#pragma pack(pop)
