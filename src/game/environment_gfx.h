#pragma once
#include "addresses.h"
#include <cstdint>

#include "log.h"
#include "data/weather_data.h"
#include "meow_hook/detour.h"
#include "meow_hook/util.h"
#include "util/hash_utils.h"

namespace gz
{
    class CEnvironmentGfxManager
    {
    public:
        char      _pad0[0xF8];              // +0x00 → +0xF7 (248 bytes)
        void**    m_weatherPresetsArray;    // +0xF8: start of weather presets array
        void**    m_weatherPresetsEnd;      // +0x100: end of weather presets array
        char      _pad1[0x8];               // +0x108 → +0x10F (8 bytes)
        uint32_t* m_weatherPresetHashes;    // +0x110: array of preset name hashes
        char      _pad2[0x20];              // +0x118 → +0x137 (32 bytes)
        int32_t   m_currentPresetIndex;     // +0x138: currently active preset index
        char      _pad3[0x1C];              // +0x13C → +0x157 (28 bytes)
        bool      m_isActive;               // +0x158: weather override active flag
        char      _pad4[0x3];               // +0x159 → +0x15B (3 bytes padding)
        int32_t   m_previousPresetIndex;    // +0x15C: previous preset index (for restore)
        // total size: 0x160
        
        static CEnvironmentGfxManager* instance()
        {
            return *(CEnvironmentGfxManager**)(GetAddress(INST_ENVIRONMENT_GFX_MANAGER));
        }

        bool SetWeatherPresetFromHash(uint32_t nameHash)
        {
            return meow_hook::func_call<bool>(
                GetAddress(SET_WEATHER_PRESET_FROM_HASH),
                this,
                nameHash
            );
        }

        bool RestoreDynamicWeather()
        {
            return meow_hook::func_call<bool>(
                GetAddress(RESTORE_DYNAMIC_WEATHER),
                this
            );
        }

        size_t GetPresetCount() const
        {
            auto start = reinterpret_cast<uintptr_t>(m_weatherPresetsArray);
            auto end   = reinterpret_cast<uintptr_t>(m_weatherPresetsEnd);
            return (end - start) / sizeof(void*);
        }

        int32_t GetCurrentPresetIndex() const {
            return m_currentPresetIndex;
        }

        int32_t FindPresetByName(const char* name) const
        {
            uint32_t hash = Utils::HashString(name);
            size_t count = GetPresetCount();

            for (size_t i = 0; i < count; i++) {
                if (m_weatherPresetHashes[i] == hash) {
                    return static_cast<int32_t>(i);
                }
            }
            return -1;  // not found
        }

        void* GetPresetByIndex(int32_t index) const
        {
            size_t count = GetPresetCount();
            if (index < 0 || static_cast<size_t>(index) >= count) {
                return nullptr; // out of bounds
            }
            return m_weatherPresetsArray[index];
        }

        // -- DATA LOOKUP HELPERS --

        static Data::Weather::WeatherPreset* GetWeatherPresetDataByHash(uint32_t hash)
        {
            for (const auto & category : Data::Weather::all_weather_presets) {
                for (size_t presetIndex = 0; presetIndex < category.count; presetIndex++) {
                    const Data::Weather::WeatherPreset& preset = category.data[presetIndex];
                    if (preset.hash == hash) {
                        return const_cast<Data::Weather::WeatherPreset*>(&preset);
                    }
                }
            }
            return nullptr;
        }

        Data::Weather::WeatherPreset* GetWeatherPresetDataByIndex(int32_t index)
        {
            size_t count = GetPresetCount();
            if (index < 0 || static_cast<size_t>(index) >= count) {
                return nullptr;
            }

            // get hash from the manager's hash array
            uint32_t presetHash = m_weatherPresetHashes[index];

            return GetWeatherPresetDataByHash(presetHash);
        }

        Data::Weather::WeatherPreset* GetCurrentWeatherPresetData()
        {
            return GetWeatherPresetDataByIndex(m_currentPresetIndex);
        }

        const char* GetCurrentWeatherPresetName()
        {
            Data::Weather::WeatherPreset* preset = GetCurrentWeatherPresetData();
            if (preset) {
                return preset->name;
            }
            return nullptr;
        }
    };
    static_assert(sizeof(CEnvironmentGfxManager) == 0x160);


    // -- WEATHER UPDATE PATCH --

    using WeatherUpdateFunc = uint64_t(__fastcall*)(CEnvironmentGfxManager* mgr, int param_2);
    static WeatherUpdateFunc g_originalWeatherUpdate = nullptr;

    static uint64_t __fastcall HookedWeatherUpdate(CEnvironmentGfxManager* mgr, int param_2)
    {
        uint64_t result = 0;

        // If we have a custom weather active AND this call is trying to disable our chosen preset
        // then block it. Otherwise allow it (for cleaning up effects, etc.)
        if (mgr->m_isActive && param_2 == mgr->m_currentPresetIndex) {
            // don't allow our custom weather to be disabled
            return result;
        }

        if (g_originalWeatherUpdate) {
            result = g_originalWeatherUpdate(mgr, param_2);
        }

        return result;
    }

    static bool SetupWeatherUpdateHook()
    {
        g_originalWeatherUpdate = MH_STATIC_DETOUR(GetAddress(WEATHER_UPDATE), HookedWeatherUpdate);
        return g_originalWeatherUpdate != nullptr;
    }
}
