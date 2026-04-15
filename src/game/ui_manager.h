#pragma once

#include "addresses.h"

#pragma pack(push, 1)
namespace gz
{
    class CUIManager
    {
    public:
        char            _pad1[0x182];   // 0x000 → 0x182
        unsigned char   m_RenderUI;     // 0x182 → 0x183

        static CUIManager* instance()
        {
            return *(CUIManager**)GetAddress(INST_UI_MANAGER);
        }


        void SetUIVisible(bool visible)
        {
            m_RenderUI = visible ? 1 : 0;
        }

        [[nodiscard]] bool IsUIVisible() const
        {
            return m_RenderUI != 0;
        }
    };
} // namespace gz
#pragma pack(pop)