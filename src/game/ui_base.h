#pragma once

#include <cstdint>
#include "addresses.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
    class CUIBase
    {
    public:
        int64_t GetUint64Field()
        {
            return meow_hook::func_call<int64_t>(GetAddress(UI_GET_UINT64), this);
        }
    };
}
#pragma pack(pop)