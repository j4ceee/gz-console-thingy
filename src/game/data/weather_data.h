#pragma once
#include <cstdint>

#include "data.h"

// Auto-generated file - do not edit manually!
// Generated from collectible JSON files

namespace gz::Data::Weather
{
    struct WeatherPreset : public DataEntry {
        const char* internal_id;
        const uint32_t hash;
    };

    using WeatherPresets = DataCategory<WeatherPreset>;

    // natural weather states
    static const WeatherPreset weather_states[] = {
        {"Thick Fog",       "fog_thick",        0x072F4F29},
        {"Overcast",        "overcast",         0x118D3F7D},
        {"Post Rain Fog",   "post_rain_fog",    0x86F336CE},
        {"Snow",            "snow",             0xA4CA1BFB},
        {"Rain",            "rain",             0xB6720744},
        {"Windy",           "windy",            0xDC26FDAF},
    };

    // vision mode overlays (used by devices/skills)
    static const WeatherPreset vision_modes[] = {
        {"Thermal Vision",  "thermal_vision",   0x8A33980A},
        {"X-Ray Vision",    "xray_vision",      0xD6E9ABC3},
        {"Night Vision",    "night_vision",     0xDAE27366},
        {"Scouting",        "scouting",         0xF49A9883},
    };

    // visual effect filters
    static const WeatherPreset visual_effects[] = {
        {"Cartoon Effect 1",    "effect_cartoon_1",         0x29A529AF},
        {"Cartoon Effect 2",    "effect_cartoon_2",         0xA1CC23F6},
        {"Sepia Effect",        "effect_sepia",             0x9C7274F0},
        {"Vintage Effect",      "effect_vintage",           0xD49BE95D},
        {"Black & White",       "effect_black_and_white",   0xF7FAA851},
        {"Hunter Mate",         "huntermate",               0x6704AFE4},
    };

    // game state effects
    static const WeatherPreset game_effects[] = {
        {"Damage Effect",   "damage_effect",    0xB44FE4C4},
        {"Heal Effect",     "heal_effect",      0xF62A3134},
        {"End Game",        "end_game",         0x6FDF9B2C},
    };

    // environmental/location presets
    static const WeatherPreset environmental[] = {
        {"High Altitude Wind",      "altitude_wind",            0x44D48FBF},
        {"Underground Interior",    "underground_interior",     0x8435B2E0},
    };

    static const WeatherPresets all_weather_presets[] = {
        {"Weather States",  6,  weather_states},
        {"Vision Modes",    4,  vision_modes},
        {"Visual Effects",  6,  visual_effects},
        {"Game Effects",    3,  game_effects},
        {"Environmental",   2,  environmental},
    };

    static constexpr size_t all_weather_presets_count = 5;

} // namespace gz::Data::Weather