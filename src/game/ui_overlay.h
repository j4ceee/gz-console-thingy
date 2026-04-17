#pragma once

#include "addresses.h"

#pragma pack(push, 1)
namespace gz
{
    class COverlayUI
    {
    public:
        static COverlayUI* instance()
        {
            return *(COverlayUI**)GetAddress(INST_OVERLAY_UI);
        }
    };
} // namespace gz
#pragma pack(pop)