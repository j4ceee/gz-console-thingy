#pragma once

#include "addresses.h"
#include "util/game_update.h"

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
            return MH_CreateHookGZ(
                CLOCK_UPDATE_GAME,
                &Hooked_UpdateGame,
                &g_updateGame
            );
        }
    };
} // namespace gz
#pragma pack(pop)