#pragma once
#include <cstdint>
#include <ctime>
#include "addresses.h"
#include "FW1FontWrapper.h"

namespace gz
{

class EventTimePatch {
private:
    // offset of XOR ECX,ECX instruction from EVENT_SCHEDULER base
    static constexpr uintptr_t TIME_CALL_OFFSET = 0x40;

    // 33 C9 (XOR) + FF 15 .. .. .. .. (CALL) = 8 Bytes
    static constexpr size_t PATCH_SIZE = 8;
    
    inline static uint8_t s_originalBytes[PATCH_SIZE] = {};
    inline static bool s_isPatched = false;
    inline static bool s_initialized = false;
    
    // pre-calculated timestamps for each event's date ranges
    struct EventDate {
        time_t timestamp;
        const char* name;
    };
    
public:
    // event timestamps
    static constexpr time_t WINTER_TIME = 1608865200;      // Dec 25, 2020
    static constexpr time_t LUNAR_TIME = 1707015600;       // Feb 4, 2024
    static constexpr time_t SEMLA_TIME = 1582772400;       // Feb 27, 2020
    static constexpr time_t ANNIVERSARY_TIME = 1712196000; // Apr 4, 2024
    static constexpr time_t HALLOWEEN_TIME = 1667185200;   // Oct 31, 2022
    
    static bool Initialize() {
        if (s_initialized) return true;
        
        uintptr_t patchAddress = GetAddress(EVENT_SCHEDULER) + TIME_CALL_OFFSET;

        // validate address
        if (IsBadReadPtr((void*)patchAddress, 2)) return false;
        auto* bytes = (uint8_t*)patchAddress;
        if (bytes[0] != 0x33 || bytes[1] != 0xC9) {
            return false; // unexpected byte pattern
        }
        
        // save original bytes
        DWORD oldProtect;
        VirtualProtect((void*)patchAddress, PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy(s_originalBytes, (void*)patchAddress, PATCH_SIZE);
        VirtualProtect((void*)patchAddress, PATCH_SIZE, oldProtect, &oldProtect);
        
        s_initialized = true;
        return true;
    }
    
    static void SetFakeTime(time_t timestamp) {
        if (!s_initialized) return;
        
        uintptr_t patchAddress = GetAddress(EVENT_SCHEDULER) + TIME_CALL_OFFSET;
        
        // build the patch:
        // MOV EAX, timestamp (5 bytes): B8 XX XX XX XX
        // NOP NOP NOP (3 bytes): 90 90 90
        uint8_t patch[PATCH_SIZE] = {
            0xB8,                                           // MOV EAX,
            (uint8_t)(timestamp & 0xFF),                    // imm32 byte 1
            (uint8_t)((timestamp >> 8) & 0xFF),             // imm32 byte 2
            (uint8_t)((timestamp >> 16) & 0xFF),            // imm32 byte 3
            (uint8_t)((timestamp >> 24) & 0xFF),            // imm32 byte 4
            0x90, 0x90, 0x90                                // NOP NOP NOP
        };
        
        DWORD oldProtect;
        VirtualProtect((void*)patchAddress, PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy((void*)patchAddress, patch, PATCH_SIZE);
        VirtualProtect((void*)patchAddress, PATCH_SIZE, oldProtect, &oldProtect);
        
        s_isPatched = true;
    }
    
    static void RestoreOriginal() {
        if (!s_initialized || !s_isPatched) return;
        
        uintptr_t patchAddress = GetAddress(EVENT_SCHEDULER) + TIME_CALL_OFFSET;
        
        DWORD oldProtect;
        VirtualProtect((void*)patchAddress, PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy((void*)patchAddress, s_originalBytes, PATCH_SIZE);
        VirtualProtect((void*)patchAddress, PATCH_SIZE, oldProtect, &oldProtect);
        
        s_isPatched = false;
    }
    
    static bool IsPatched() {
        return s_isPatched;
    }
};

}