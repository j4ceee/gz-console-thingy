#pragma once

#include <cstdint>
#include "addresses.h"
#include "meow_hook/detour.h"

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

    class CSpawnedAnimalNetworkComponent
    {
    public:
        char            _pad0[0x5D8];       // 0x000 → 0x5D8
        SHackedState    m_HackedState;      // 0x5D8 → 0x5E8 (embedded struct, not pointer)
        char            _pad1[0x20];        // 0x5E8 → ...

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

            g_requestHack(comp, requestData);
        }

        static bool SetupHackRequestHook()
        {
            if (g_requestHack != nullptr) {
                return true; // already set up
            }
            g_requestHack = MH_STATIC_DETOUR(GetAddress(REQUEST_ANIMAL_HACK), HookedRequestHack);
            return g_requestHack != nullptr;
        }
    };
} // namespace gz
#pragma pack(pop)