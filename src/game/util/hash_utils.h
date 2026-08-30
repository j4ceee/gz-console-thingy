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
        if (g_hashString)
        {
            return true; // already set up
        }
        g_hashString = (HashStringFunc)GetAddress(HASHING_FUNC);
        return g_hashString != nullptr;
    }

    inline uint32_t HashString(const char* str)
    {
        if (g_hashString)
        {
            return g_hashString(str);
        }
        return 0;
    }


    using HashBufferFunc = uint32_t(__fastcall*)(const uint8_t* buf, uint64_t len, uint32_t seed);
    inline HashBufferFunc g_hashBuffer = nullptr;

    inline bool SetupHashBufferFunction()
    {
        if (g_hashBuffer) return true;
        g_hashBuffer = (HashBufferFunc)GetAddress(HASHING_FUNC_BUFF);
        return g_hashBuffer != nullptr;
    }

    inline uint32_t HashBuffer(const char* str, uint32_t seed = 0)
    {
        if (g_hashBuffer)
        {
            return g_hashBuffer(reinterpret_cast<const uint8_t*>(str), strlen(str), seed);
        }
        return 0;
    }
} // namespace gz::Utils
#pragma pack(pop)
