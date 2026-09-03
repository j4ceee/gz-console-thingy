#pragma once

#include <cstdint>
#include "meow_hook/util.h"
#include "addresses.h"
#include "data/weather_data.h"
#include "util/structs.h"

#pragma pack(push, 1)
namespace gz
{
    struct SParamSetContext
    {
        Utils::SResourceInfo*   m_Parameters;   // +0x00
        uint32_t                m_ID;           // +0x08
        float                   m_BlendValue;   // +0x0C
        float                   m_TotalBlendTime; // +0x10
        float                   m_TimeBlend;    // +0x14
        uint8_t                 m_Flags;        // +0x18
        uint8_t                 _pad0[3];       // +0x19
        uint32_t                m_Priority;     // +0x1C
    };
    static_assert(sizeof(SParamSetContext) == 0x20);

    class CEnvironmentGfxManager
    {
    public:
        char                _pad0[0x68];            // 0x000
        SParamSetContext*   m_paramSetsBegin;       // 0x068
        SParamSetContext*   m_paramSetsEnd;         // 0x070
        char                _pad1[0x80];            // 0x078
        void**              m_weatherPresetsArray;  // 0x0F8: start of weather presets array
        void**              m_weatherPresetsEnd;    // 0x100: end of weather presets array
        char                _pad2[0x8];             // 0x108 → 0x10F (8 bytes)
        uint32_t*           m_weatherPresetHashes;  // 0x110: array of preset name hashes
        char                _pad3[0x20];            // 0x118 → 0x137 (32 bytes)
        int32_t             m_currentPresetIndex;   // 0x138: currently active preset index
        char                _pad4[0x1C];            // 0x13C → 0x157 (28 bytes)
        bool                m_isActive;             // 0x158: weather override active flag
        char                _pad5[0x3];             // 0x159 → 0x15B (3 bytes padding)
        int32_t             m_previousPresetIndex;  // 0x15C: previous preset index (for restore)

        static CEnvironmentGfxManager* instance()
        {
            return *(CEnvironmentGfxManager**)(GetAddress(INST_ENVIRONMENT_GFX_MANAGER));
        }

        // blendTime < 0 uses the game's default (g_PresetsBlendingTime)
        void AddParameterSet(void** presetHandle, float blend, uint32_t id, uint32_t priority, float blendTime = -1.0f)
        {
            meow_hook::func_call<void>(GetAddress(GFX_ADD_PARAM_SET), this, presetHandle, blend, id, priority, blendTime);
        }

        // flags the set for removal; it fades back out over its blend time
        bool RemoveParameterSet(uint32_t id)
        {
            return meow_hook::func_call<bool>(GetAddress(GFX_REMOVE_PRESET), this, id);
        }

        [[nodiscard]] size_t GetPresetCount() const
        {
            auto start = reinterpret_cast<uintptr_t>(m_weatherPresetsArray);
            auto end   = reinterpret_cast<uintptr_t>(m_weatherPresetsEnd);
            return (end - start) / sizeof(void*);
        }

        [[nodiscard]] void** GetPresetHandle(int32_t index) const
        {
            if (index < 0 || static_cast<size_t>(index) >= GetPresetCount())
                return nullptr;
            return &m_weatherPresetsArray[index];
        }

        [[nodiscard]] int32_t FindPresetIndexByHash(uint32_t hash) const
        {
            size_t count = GetPresetCount();
            for (size_t i = 0; i < count; i++) {
                if (m_weatherPresetHashes[i] == hash)
                    return static_cast<int32_t>(i);
            }
            return -1;
        }

        // -- PARAM SET STACK INSPECTION --

        [[nodiscard]] size_t GetParamSetCount() const { return m_paramSetsEnd - m_paramSetsBegin; }

        [[nodiscard]] const SParamSetContext* FindParamSet(uint32_t id) const
        {
            for (auto* p = m_paramSetsBegin; p != m_paramSetsEnd; ++p) {
                if (p->m_ID == id && !(p->m_Flags & 2))   // ignore ones fading out
                    return p;
            }
            return nullptr;
        }

        [[nodiscard]] bool HasParamSet(uint32_t id) const { return FindParamSet(id) != nullptr; }

        // -- DATA LOOKUP HELPERS --

        static Data::Weather::WeatherPreset* GetWeatherPresetDataByHash(uint32_t hash)
        {
            for (const auto & category : Data::Weather::all_presets) {
                for (size_t presetIndex = 0; presetIndex < category.count; presetIndex++) {
                    const Data::Weather::WeatherPreset& preset = category.data[presetIndex];
                    if (preset.hash == hash) {
                        return const_cast<Data::Weather::WeatherPreset*>(&preset);
                    }
                }
            }
            return nullptr;
        }
    };
    static_assert(sizeof(CEnvironmentGfxManager) == 0x160);
    static_assert(offsetof(CEnvironmentGfxManager, m_weatherPresetsArray) == 0xF8);
    static_assert(offsetof(CEnvironmentGfxManager, m_isActive) == 0x158);
} // namespace gz
#pragma pack(pop)
