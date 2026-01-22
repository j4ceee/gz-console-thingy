#pragma once
#include "../../addresses.h"
#include <cstdint>

#pragma pack(push, 1)
namespace gz::Utils
{
using HashStringFunc = uint32_t(__fastcall*)(const char* str);
inline HashStringFunc g_hashString = nullptr;

inline bool SetupHashFunction()
{
    if (g_hashString) {
        return true; // already set up
    }
    g_hashString = (HashStringFunc)GetAddress(HASHING_FUNC);
    return g_hashString != nullptr;
}

inline uint32_t HashString(const char* str)
{
    if (g_hashString) {
        return g_hashString(str);
    }
    return 0;
}
} // namespace gz::Utils
#pragma pack(pop)