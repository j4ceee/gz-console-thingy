#pragma once
#include "addresses.h"
#include <cstdint>
#include <Windows.h>

namespace gz
{
struct PatchInfo
{
    Address             addressEnum;   // which address from addresses.h
    const uint8_t*      originalBytes; // expected original bytes
    const uint8_t*      patchBytes;    // bytes to write when enabled
    size_t              size;          // number of bytes to patch
    const char*         description;   // for debugging/logging

    // helper methods
    bool Verify(uintptr_t address) const
    {
        return memcmp((void*)address, originalBytes, size) == 0;
    }

    void Apply(uintptr_t address) const
    {
        PatchMemory(address, patchBytes, size);
    }

    void Restore(uintptr_t address) const
    {
        PatchMemory(address, originalBytes, size);
    }

private:
    static void PatchMemory(uintptr_t address, const uint8_t* bytes, size_t size)
    {
        DWORD oldProtect;
        VirtualProtect((void*)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy((void*)address, bytes, size);
        VirtualProtect((void*)address, size, oldProtect, &oldProtect);
    }
};

// helper functions for batch operations
namespace PatchUtils
{
    inline bool VerifyAll(const PatchInfo* patches, size_t count)
    {
        bool allValid = true;
        for (size_t i = 0; i < count; i++) {
            uintptr_t address = GetAddress(patches[i].addressEnum);
            if (!patches[i].Verify(address)) {
                allValid = false;
            }
        }
        return allValid;
    }

    inline void ApplyAll(const PatchInfo* patches, size_t count)
    {
        for (size_t i = 0; i < count; i++) {
            uintptr_t address = GetAddress(patches[i].addressEnum);
            patches[i].Apply(address);
        }
    }

    inline void RestoreAll(const PatchInfo* patches, size_t count)
    {
        for (size_t i = 0; i < count; i++) {
            uintptr_t address = GetAddress(patches[i].addressEnum);
            patches[i].Restore(address);
        }
    }
}
} // namespace gz