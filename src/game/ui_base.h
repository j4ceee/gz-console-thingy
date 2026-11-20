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
        [[nodiscard]] int64_t GetUint64Field() const
        {
            return meow_hook::func_call<int64_t>(GetAddress(UI_GET_UINT64), this);
        }

        /// Sets a float member on a Scaleform value object (e.g. the HUD's m_VehicleData).
        static void SetFloatField(void* value, const char* name, float number)
        {
            meow_hook::func_call<void>(GetAddress(UI_SET_FLOAT_FIELD), value, name, number);
        }

        /// Sets an int member on a Scaleform value object (e.g. the HUD's m_VehicleData).
        static void SetIntField(void* value, const char* name, int number)
        {
            meow_hook::func_call<void>(GetAddress(UI_SET_INT_FIELD), value, name, number);
        }
    };
}
#pragma pack(pop)