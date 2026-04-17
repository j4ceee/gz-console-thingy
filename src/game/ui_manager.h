#pragma once

#include "addresses.h"
#include "ui_static_handler.h"

#pragma pack(push, 1)
namespace gz
{
    class CUIManager;

    using IsUIShownFunc = bool(*)(CUIManager*, CHashString*);
    inline IsUIShownFunc g_isUIShown = nullptr;

    class CUIManager
    {
    public:
        char            _pad1[0x182];   // 0x000 → 0x182
        unsigned char   m_RenderUI;     // 0x182 → 0x183 - toggle entire UI rendering

        static CUIManager* instance()
        {
            return *(CUIManager**)GetAddress(INST_UI_MANAGER);
        }

        /// <summary>
        /// Enable / disable entire UI rendering (menus, HUD, ...)
        /// </summary>
        void SetUIVisible(bool visible)
        {
            m_RenderUI = visible ? 1 : 0;
        }

        /// <summary>
        /// Check if the entire UI rendering is enabled (menus, HUD, ...)
        /// </summary>
        [[nodiscard]] bool IsUIVisible() const
        {
            return m_RenderUI != 0;
        }

        static bool IsHudHidden()
        {
            return g_hideHud;
        }

        static void SetHideHud(bool hide)
        {
            g_hideHud = hide;
        }

        static bool HookedIsUIShown(CUIManager* mgr, CHashString* id)
        {
            if (g_isUIShown)
            {
                if (g_hideHud && id->m_Hash == KnownUIHashes::HUD && g_hudIsVisible)
                    return true;

                return g_isUIShown(mgr, id);
            }
            return false;
        }

        static bool SetupIsUIShownHook()
        {
            if (g_isUIShown != nullptr) {
                return true;
            }
            return MH_CreateHookGZ(FUNC_IS_UI_SHOWN, &HookedIsUIShown, &g_isUIShown);
        }
    };
} // namespace gz
#pragma pack(pop)