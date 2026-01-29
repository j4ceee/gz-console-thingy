#pragma once

#include <cstdint>
#include "addresses.h"
#include "animal.h"
#include "hook_helpers.h"
#include "imgui/ui.h"

#pragma pack(push, 1)
namespace gz
{
    class CSpawnedAnimalNetworkComponent;

    struct SHackRequestData {
        uint64_t unknown1;              // 0x00 → 0x08
        uint32_t hackDurationMs;        // 0x08 → 0x0C - skill bonus * 1000
        float    hackSuccessProbability;// 0x0C → 0x10 - from m_HackSuccessProbability
        uint8_t  hasAdvancedHackSkill;  // 0x10 → 0x11 - skill 0x2acb1485 active
        uint8_t  padding[7];            // 0x11 → 0x18
    };

    struct SHackedState {
        uint8_t  m_IsHacked;         // +0x5D8
        uint8_t  m_IsAdvancedHack;   // +0x5D9
        uint16_t _pad;               // +0x5DA
        uint32_t m_HackedUntilTime;  // +0x5DC (network time + duration)
        uint64_t m_HackerInfo;       // +0x5E0
    };

    using RequestHackFunc = void(*)(CSpawnedAnimalNetworkComponent* comp, SHackRequestData* requestData);
    inline RequestHackFunc g_requestHack = nullptr;

    inline bool g_takeControlOnHack = false;

    class CSpawnedAnimalNetworkComponent
    {
    public:
        char            _pad0[0x5D8];       // 0x000 → 0x5D8
        SHackedState    m_HackedState;      // 0x5D8 → 0x5E8 (embedded struct, not pointer)
        CAnimal*        m_Animal;           // 0x5E8 → 0x5F0

        CAnimal* GetAnimal() const
        {
            return m_Animal;
        }

        CCharacter* GetCharacter() const
        {
            if (!m_Animal) return nullptr;
            return m_Animal->GetSpawnedCharacter();
        }

        bool IsHacked() const
        {
            return m_HackedState.m_IsHacked != 0;
        }

        static void HookedRequestHack(CSpawnedAnimalNetworkComponent* comp, SHackRequestData* requestData)
        {
            if (g_requestHack == nullptr) {
                return;
            }

            UI* ui = UI::Get();
            ConsoleSettings& settings = ui->GetSettings();

            if (settings.hackingAlwaysSucceeds)
                requestData->hackSuccessProbability = 10.0f;
            if (settings.unlimitedHackingTime)
                requestData->hackDurationMs = 10000000; // 2.7 hours
            if (settings.alwaysAdvancedHacking)
                requestData->hasAdvancedHackSkill = 1;

            bool isAdvancedHack = requestData->hasAdvancedHackSkill != 0;

            g_requestHack(comp, requestData);

            CCharacter* mChar = comp->GetCharacter();

            // only take control if...
            // - the option is enabled
            // - advanced hack (hacker skill or always advanced hack) is used
            // - machine character exists
            // - hack was successful
            // - machine is not soviet (issues with controls)
            if (g_takeControlOnHack && isAdvancedHack && mChar && !mChar->IsSoviet() && comp->IsHacked())
            {
                CCharacter* playerChar = CNetworkPlayerManager::instance()->GetPlayer()->GetCharacter();
                CRemoteController* rc = playerChar ? playerChar->GetRemoteController() : nullptr;
                if (rc && playerChar->IsWithinInteractionRangeOf(mChar))
                {
                    rc->RequestControlOfCharacter(mChar);
                }
            }
        }

        static bool SetupHackRequestHook()
        {
            if (g_requestHack != nullptr) {
                return true; // already set up
            }
            return MH_CreateHookGZ(REQUEST_ANIMAL_HACK, &HookedRequestHack, &g_requestHack);
        }
    };
} // namespace gz
#pragma pack(pop)