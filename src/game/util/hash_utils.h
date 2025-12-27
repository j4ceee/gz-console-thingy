#pragma once
#include "../../addresses.h"
#include <cstdint>

namespace gz::Utils
{
using HashStringFunc = uint32_t(__fastcall*)(const char* str);
inline HashStringFunc g_hashString = nullptr;

inline HashStringFunc SetupHashFunction()
{
    if (!g_hashString) {
        g_hashString = (HashStringFunc)GetAddress(HASHING_FUNC);
    }
    return g_hashString;
}

inline uint32_t HashString(const char* str)
{
    if (!g_hashString) {
        SetupHashFunction();
    }
    if (g_hashString) {
        return g_hashString(str);
    }
    return 0;
}
}