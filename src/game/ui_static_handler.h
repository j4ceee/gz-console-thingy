#pragma once

#include "addresses.h"

#pragma pack(push, 1)
namespace gz
{
    class CUIStaticHandler
    {
    public:
        char        _pad1[0x8]; // 0x00 → 0x08
        uint32_t    m_Hash;     // 0x08 → 0x0C
    };


    struct KnownUIHashes
    {
        static constexpr uint32_t EMOTE_WHEEL = 0xB4C66E80;
        static constexpr uint32_t WEAPON_WHEEL = 0xDCE8BF98;
        static constexpr uint32_t HUD = 0x97B511F1;
    };

    struct SUIScene;
    using UpdateVisibilityShowFunc = void(*)(SUIScene*, bool);
    inline UpdateVisibilityShowFunc g_updVisShow = nullptr;

    inline bool g_hudIsVisible = false; // hud is visible internally
    inline bool g_hideHud = false; // user wants to hide hud

    struct SUIScene
    {
    public:
        char                _pad1[0x88];    // 0x00 → 0x88
        CUIStaticHandler*   m_Handler;      // 0x88 → 0x90
        char                _pad2[0x08];    // 0x90 → 0x98
        uint32_t            m_ID;           // 0x98 → 0x9C
        uint32_t            m_State;        // 0x9C → 0xA0

        static void HookedUpdateVisibilityShow(SUIScene* scene, bool show)
        {
            if (g_updVisShow)
            {
                g_updVisShow(scene, show);
            }

            if (g_hideHud && scene->m_Handler != nullptr && scene->m_Handler->m_Hash == KnownUIHashes::HUD)
            {
                if (scene->m_State == 2)
                {
                    g_hudIsVisible = true;
                    scene->m_State = 1;
                } else
                {
                    g_hudIsVisible = false;
                }
            }
        }

        static bool SetupUpdateVisibilityShowHook()
        {
            if (g_updVisShow != nullptr) {
                return true;
            }
            return MH_CreateHookGZ(FUNC_UPD_VIS_SHOW, &HookedUpdateVisibilityShow, &g_updVisShow);
        }
    };
} // namespace gz
#pragma pack(pop)