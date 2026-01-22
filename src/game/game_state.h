#pragma once

#include <cstdint>
#include "addresses.h"

#pragma pack(push, 1)
namespace gz::GameState
{
    // frontend
    const uint32_t AWAIT_UNLOAD =       0;
    const uint32_t LOAD_GLOBAL =        1;
    const uint32_t PRESS_START =        2;
    const uint32_t LOGIN =              3;
    const uint32_t APPLY_GLOBAL_STATE = 4;
    const uint32_t LOAD_DESTINATION =   5;
    const uint32_t LOGOUT =             6;
    const uint32_t LOAD_RESERVE =       7;
    const uint32_t LOAD_SPAWN =         8;
    const uint32_t LOAD_WAIT =          9;
    const uint32_t MAIN_MENU =          10;
    const uint32_t DONE =               11;
    const uint32_t QUIT =               12;
    const uint32_t AVALANCHE_ACCOUNT =  13;

    // game state
    const uint32_t E_GAME_INSTALL =     0;
    const uint32_t E_GAME_INIT =        1;
    const uint32_t E_GAME_FRONTEND =    2;
    const uint32_t E_GAME_LOAD =        3;
    const uint32_t E_GAME_RUN =         4;

    static uint32_t GetFrontendState()
    {
        return *(uint32_t *)GetAddress(VAR_GAME_STATE_FRONTEND);
    }

    static bool IsInFrontend()
    {
        uint32_t state = GetFrontendState();
        return (state != DONE);
    }

    static uint32_t GetGameState()
    {
        return *(uint32_t *)GetAddress(VAR_GAME_STATE);
    }

    static bool IsInGame()
    {
        uint32_t state = GetGameState();
        uint32_t frontendState = GetFrontendState();
        return (state == E_GAME_RUN && frontendState == DONE);
    }

} // namespace gz
#pragma pack(pop)