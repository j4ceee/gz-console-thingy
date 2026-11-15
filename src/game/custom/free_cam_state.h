#pragma once

namespace gz::FreeCamState
{
    inline bool g_enabled = false; // requested (hotkey / UI checkbox). never persisted
    inline bool g_active = false;  // camera hook has actually taken over
    inline bool g_parked = false;  // player character is parked (set / cleared on the game tick)

    /// true from the request until the player is back in the world. use this to guard gameplay actions
    [[nodiscard]] inline bool IsInUse() { return g_enabled || g_active || g_parked; }
    inline bool IsEnabled() { return g_enabled; }
    inline bool IsActive() { return g_active; }
}