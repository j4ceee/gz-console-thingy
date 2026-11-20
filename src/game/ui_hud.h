#pragma once

#include <algorithm>
#include <intrin.h>

#include "addresses.h"
#include "hook_helpers.h"
#include "player.h"
#include "player_network_manager.h"
#include "settings_manager.h"
#include "ui_base.h"
#include "custom/free_cam_state.h"

#pragma pack(push, 1)
namespace gz
{
    class CHUDUI;

    using HudUpdateStateFunc = void(*)(CHUDUI*, CPlayer*);
    inline HudUpdateStateFunc g_hudUpdateState = nullptr;
    using HudSetMemberFunc = bool(*)(void*, void*, const char*, void*, bool);
    inline HudSetMemberFunc g_hudSetMember = nullptr;

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

        static bool SetupVehicleDataHook()
        {
            if (g_hudSetMember != nullptr) return true;
            return MH_CreateHookGZ(UI_SET_MEMBER, &HookedSetMember, &g_hudSetMember);
        }

    private:
        static bool HookedSetMember(void* objectInterface, void* object, const char* name, void* value, bool isDisplayObject)
        {
            // only update vehicle data object committed by CHUDUI::UpdatePlayerVehicle
            if (reinterpret_cast<uintptr_t>(_ReturnAddress()) == GetAddress(HUD_VEHICLE_DATA_RETURN))
            {
                auto* vehicle = CCharacter::GetVehicle();
                if (vehicle && vehicle->IsCar() && vehicle->HasPhysicsVehicle())
                    SetCarVehicleData(value, vehicle);
            }
            return g_hudSetMember(objectInterface, object, name, value, isDisplayObject);
        }

        static void SetCarVehicleData(void* value, CVehicle* vehicle)
        {
            auto* damage = vehicle->GetDamageable();
            const int maxHealth = damage->GetMaxHealth();
            const float health = maxHealth > 0
                                     ? std::clamp(static_cast<float>(damage->GetHealth()) / maxHealth, 0.0f, 1.0f)
                                     : 0.0f;
            auto* settings = CSettingsManager::instance();
            const int units = settings ? settings->GetDisplayUnits() : 0;
            const float speedScale = units == 0 ? 3.6f : units == 1 ? 2.24f : 1.0f;

            CUIBase::SetFloatField(value, "m_Health", health);
            CUIBase::SetFloatField(value, "m_TargetHealth", health);
            CUIBase::SetIntField(value, "m_FuelLevel", static_cast<int>(std::clamp(vehicle->GetFuelLevel(), 0.0f, 100.0f)));
            CUIBase::SetIntField(value, "m_Speed", static_cast<int>(vehicle->GetSpeedMs() * speedScale));
            CUIBase::SetIntField(value, "m_TopSpeed", static_cast<int>(vehicle->GetOfficialTopSpeedMs() * speedScale));
        }
    };
} // namespace gz
#pragma pack(pop)
