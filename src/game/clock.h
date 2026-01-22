#pragma once

#include "addresses.h"
#include "util/game_update.h"
#include "meow_hook/detour.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
    using UpdateGame = void(*)(void* clockInstance);
    static UpdateGame g_updateGame = nullptr;

    class CClock
    {
    public:
        static CClock* instance()
        {
            return *(CClock**)(GetAddress(INST_CLOCK));
        }

        static void Hooked_UpdateGame(void* clockInstance)
        {
            // call original function
            if (g_updateGame)
            {
                g_updateGame(clockInstance);
            }

            Utils::OnGameUpdate();
        }

        static bool SetupUpdateGameHook()
        {
            if (g_updateGame != nullptr) {
                return true; // already set up
            }
            g_updateGame = MH_STATIC_DETOUR(GetAddress(CLOCK_UPDATE_GAME), Hooked_UpdateGame);
            return g_updateGame != nullptr;
        }
    };
} // namespace gz
#pragma pack(pop)