#pragma once
#include <algorithm>

#include "addresses.h"
#include "log.h"
#include "meow_hook/detour.h"
#include <cstdarg>

#pragma pack(push, 1)
namespace gz
{

    using VsnprintfInternalFunc = int64_t(__fastcall*)(char* buffer, size_t size, const char* format, va_list args);
    inline VsnprintfInternalFunc g_origVsnprintf = nullptr;

    using ScriptErrorFunc = void(__fastcall*)(int64_t ctx, const char* errorMsg, int64_t arg1, int64_t arg2);
    inline ScriptErrorFunc g_origScriptError = nullptr;

    inline thread_local bool t_suppressVsnprintfLog = false;

    inline int64_t __fastcall HookedVsnprintfInternal(char* buffer, size_t size, const char* format, va_list args) {
        int64_t result = g_origVsnprintf(buffer, size, format, args);

        if (t_suppressVsnprintfLog) {
            return result;
        }

        if (result > 0 && buffer != nullptr && static_cast<size_t>(result) < size) {
            static const char* keywords[] = {
                "couldn",
                "unable",
                "error",
                "failed",
            };

            static const char* keywords_ignore[] = {
                ".com",
                "[send]",
            };

            std::string lowerBuffer = buffer;
            std::transform(lowerBuffer.begin(), lowerBuffer.end(), lowerBuffer.begin(), ::tolower);

            for (const char* keyword : keywords_ignore) {
                if (lowerBuffer.find(keyword) != std::string::npos) {
                    return result;
                }
            }
            
            for (const char* keyword : keywords) {
                if (lowerBuffer.find(keyword) != std::string::npos) {
                    Log("[Game Output] %s", buffer);
                    break;
                }
            }
        }

        return result;
    }

    inline void __fastcall HookedScriptError(int64_t ctx, const char* errorMsg, int64_t arg1, int64_t arg2) {
        t_suppressVsnprintfLog = true;

        char formatBuffer[1024];
        if (errorMsg) {
            snprintf(formatBuffer, sizeof(formatBuffer), errorMsg, arg1, arg2);
            Log("[Script Error] %s", formatBuffer);
        } else {
            Log("[Script Error] <Null Message>");
        }

        g_origScriptError(ctx, errorMsg, arg1, arg2);

        t_suppressVsnprintfLog = false;
    }

    inline void SetupLoggingHooks() {
        g_origVsnprintf = MH_STATIC_DETOUR(GetAddress(FUNC_VSNPRINTF), HookedVsnprintfInternal);
        g_origScriptError = MH_STATIC_DETOUR(GetAddress(FUNC_SCRIPT_ERROR_REPORT), HookedScriptError);
        
        Log("Logging hooks initialized. monitoring game output...");
    }
}
#pragma pack(pop)