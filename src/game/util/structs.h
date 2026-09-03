#pragma once

#include <cstdint>

#pragma pack(push, 1)
namespace gz::Utils
{
    struct SResourceInfo // 0x30
    {
        void*   m_Resource;         // +0x00
        void*   m_UserCtx;          // +0x08
        void*   m_ResourceCache;    // +0x10
        uint32_t m_ExtensionHash;   // +0x18
        uint32_t m_PathHash;        // +0x1C
        uint32_t m_RefCount;        // +0x20
        uint8_t m_LockCount;        // +0x24
        uint8_t _pad0[3];
        void*   m_ReloadCallbacks;  // +0x28

        [[nodiscard]] void* GetResource()
        {
            return m_Resource;
        }
    };
}
#pragma pack(pop)