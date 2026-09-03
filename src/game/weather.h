#pragma once

#include <cstdint>
#include "addresses.h"

#pragma pack(push, 1)
namespace gz
{
    struct SWindData
    {
        float m_WindBaseDirectionDeg;       // +0x00
        float m_WindDirectionDeg;           // +0x04
        char  _pad0[0x14];                  // +0x08
        float m_WindStrengthAtCameraPos;    // +0x1C
        float m_GustMapCoverage;            // +0x20
        char  _pad1[0x04];                  // +0x24
        float m_GustMapStrengthMin;         // +0x28
        float m_GustMapStrengthMax;         // +0x2C
        float m_GustMapVelocity;            // +0x30
    };
    static_assert(sizeof(SWindData) == 0x34);
    static_assert(offsetof(SWindData, m_WindStrengthAtCameraPos) == 0x1C);
    static_assert(offsetof(SWindData, m_GustMapCoverage) == 0x20);
    static_assert(offsetof(SWindData, m_GustMapStrengthMin) == 0x28);
    static_assert(offsetof(SWindData, m_GustMapStrengthMax) == 0x2C);
    static_assert(offsetof(SWindData, m_GustMapVelocity) == 0x30);

    class CPrecipitation
    {
    public:
        float   m_Intensity;        // +0x00
        float   m_ConstantAmount;   // +0x04
        float   m_SnowRatio;        // +0x08
        char    _pad0[0x30];        // +0x0C
        bool    m_UserControlled;   // +0x3C blocks CWeather::PreUpdate writes
        bool    m_Enabled;          // +0x3D
        char    _pad1[0x2A];        // +0x3E
    };
    static_assert(sizeof(CPrecipitation) == 0x68);
    static_assert(offsetof(CPrecipitation, m_UserControlled) == 0x3C);

    class CWeather
    {
    public:
        char            _pad0[0x90];                    // +0x000
        float           m_DrawDistance;                 // +0x090
        float           m_CloudBase;                    // +0x094 - altitude where clouds start
        float           m_CloudHeight;                  // +0x098 - vertical thickness of clouds
        float           m_PrevCloudBase;                // +0x09C
        float           m_CirrusCloudBase;              // +0x0A0
        float           m_CirrusCloudHeight;            // +0x0A4
        void*           m_CloudFractal;                 // +0x0A8
        void*           m_CloudOvercastFractal;         // +0x0B0
        bool            m_CloudShadowEnabled;           // +0x0B8
        char            _pad01[0x3];                    // +0x0B9
        float           m_CloudDiffuseColor[3];         // +0x0BC
        float           m_CloudAmbientColor[3];         // +0x0C8
        float           m_CloudTranslucencyMultiplier;  // +0x0D4
        float           m_CloudDiffuseMultiplier;       // +0x0D8
        float           m_CloudAmbientMultiplier;       // +0x0DC
        float           m_CloudCoverage;                // +0x0E0
        float           m_NextCloudCoverage;            // +0x0E4
        char            _pad1[0x8];                     // +0x0E8
        CPrecipitation* m_Precipitation;                // +0x0F0
        float           m_SnowRatio;                    // +0x0F8
        float           m_RainIntensity;                // +0x0FC
        float           m_Wetness;                      // +0x100
        float           m_MinimumWetness;               // +0x104
        float           m_MaximumWetness;               // +0x108
        float           m_SnowAmount;                   // +0x10C
        float           m_SnowRandomness;               // +0x110
        float           m_MinimumSnowAmount;            // +0x110
        float           m_MaximumSnowAmount;            // +0x114
        float           m_WetSpeed;                     // +0x11C
        float           m_DrySpeed;                     // +0x120
        float           m_PrecipitationThreshold;       // +0x124
        char            _pad2[0xC];                     // +0x128
        float           m_LightningIntensity;           // +0x134
        float           m_LightningIntensityMultiplier; // +0x138
        float           m_LightningPossibility;         // +0x13C
        char            _pad3[0xC];                     // +0x140
        char            _pad4[0x78];                    // +0x14C
        float           m_AirDensity;                   // +0x1C4
        float           m_CurrentDensity;               // +0x1C8
        float           m_NextDensity;                  // +0x1CC
        char            _pad5[0xC];                     // +0x1D0
        float           m_WeatherTime;                  // +0x1DC
        float           m_Acceleration;                 // +0x1E0
        float           m_Severity;                     // +0x1E4
        float           m_SeverityTarget;               // +0x1E8
        float           m_SeverityUpdateTime;           // +0x1EC
        float           m_SeverityMin;                  // +0x1F0
        float           m_SeverityMax;                  // +0x1F4
        float           m_SeverityMinMaxCurve;          // +0x1F8
        float           m_SeverityTransitionFrequency;  // +0x1FC
        float           m_SeverityTransitionTime;       // +0x200
        float           m_WindDirectionDeviationDeg;    // +0x204
        float           m_WindDirectionDeviationFreq;   // +0x208
        char            _pad6[0x8];                     // +0x20C
        bool            m_OverrideWindStrength;         // +0x214
        char            _pad7[0x3];                     // +0x215
        float           m_Altitude;                     // +0x218
        float           m_DeltaTime;                    // +0x21C
        int32_t         m_ForceUpdate;                  // +0x220
        bool            m_HasUpdated;                   // +0x224
        bool            m_DoUpdateSeverity;             // +0x225
        char            _pad8[0x2];                     // +0x226
        char            _pad9[0xC];                     // +0x228
        bool            m_ForceLowVisuals;              // +0x234
        bool            m_IsNetworkControlled;          // +0x235
        char            _pad10[0x2];                    // +0x236
        float           m_Visibility;                   // +0x238
        uint32_t        m_ActiveReserve;                // +0x23C
        char            _pad11[0x28];                   // +0x240
        SWindData       m_WindData;                     // +0x268
        char            _pad12[0x44];                   // +0x29C
        bool            m_Enabled;                      // +0x2E0
        char            _pad13[0x67];                   // +0x2E1
        void**          m_WeatherPresetsArray;          // +0x348
        void**          m_WeatherPresetsEnd;            // +0x350
        char            _pad14[0x8];

        [[nodiscard]] CPrecipitation* GetPrecipitation() const
        {
            return m_Precipitation;
        }
    };
    static_assert(sizeof(CWeather) == 0x360);
    static_assert(offsetof(CWeather, m_CloudBase) == 0x094);
    static_assert(offsetof(CWeather, m_CloudHeight) == 0x098);
    static_assert(offsetof(CWeather, m_CloudCoverage) == 0x0E0);
    static_assert(offsetof(CWeather, m_Precipitation) == 0x0F0);
    static_assert(offsetof(CWeather, m_SnowAmount) == 0x10C);
    static_assert(offsetof(CWeather, m_MinimumSnowAmount) == 0x114);
    static_assert(offsetof(CWeather, m_MaximumSnowAmount) == 0x118);
    static_assert(offsetof(CWeather, m_WetSpeed) == 0x11C);
    static_assert(offsetof(CWeather, m_Severity) == 0x1E4);
    static_assert(offsetof(CWeather, m_SeverityMin) == 0x1F0);
    static_assert(offsetof(CWeather, m_ForceUpdate) == 0x220);
    static_assert(offsetof(CWeather, m_ActiveReserve) == 0x23C);
    static_assert(offsetof(CWeather, m_Enabled) == 0x2E0);
    static_assert(offsetof(CWeather, m_WeatherPresetsArray) == 0x348);

    class CAtmosphere
    {
    public:
        char        _pad0[0x12];    // +0x000
        bool        m_Initalized;   // +0x012
        bool        m_RenderWeather;// +0x013
        char        _pad1[0x4F4];   // +0x014
        CWeather*   m_Weather;      // +0x508

        [[nodiscard]] bool IsInitialized() const
        {
            return m_Initalized;
        }

        [[nodiscard]] bool RenderWeather() const
        {
            return m_RenderWeather;
        }

        [[nodiscard]] CWeather* GetWeather() const
        {
            return m_Weather;
        }
    };
    static_assert(offsetof(CAtmosphere, m_Weather) == 0x508);

    namespace Weather
    {
        using PreUpdateFunc = void(__fastcall*)(CWeather* weather, bool paused, float deltaTime, void* camera);
        using UpdateFunc = void(__fastcall*)(CWeather* weather, float param1, bool param2, float param3, void* param4, bool param5);

        inline PreUpdateFunc g_originalPreUpdate = nullptr;
        inline UpdateFunc g_originalUpdate = nullptr;

        struct Overrides
        {
            bool enabled = false;

            float cloudDensity = 1.0f; // m_Severity target, natural range ~1.6 - 6
            float transitionTime = 1.0f; // seconds for cloud density to ease in

            bool overrideDownpour = true; // take CPrecipitation off the cloud sim
            float downpour = 0.0f; // 0 - 1 precipitation particle density

            float groundWetness = 0.20f; // pins m_Wetness floor
            float lightningChance = 0.81f; // >1 is valid

            float snowRatio = 0.0f; // >0.5 = snow, <0.5 = rain
            float snowRandomness = 0.75f;
            float groundSnow = 0.03f; // pins m_SnowAmount

            float cloudHeight = 1500.0f;
            float cloudThickness = 1000.0f;

            float gustStrength = 4.0f;
        };

        inline Overrides g_overrides;
        inline bool g_precipOwned = false;

        /// Writes custom values into CWeather before the original runs, so everything downstream uses our custom values.
        /// Game keeps maintaining real values the whole time in the background, so we can go back to vanilla at any time
        inline void ApplyOverrides(CWeather* weather)
        {
            if (!weather) return;

            if (!g_overrides.enabled)
            {
                // hand precipitation back if we were holding it
                if (g_precipOwned && weather->m_Precipitation)
                {
                    weather->m_Precipitation->m_UserControlled = false;
                    g_precipOwned = false;
                }
                return;
            }

            // severity drives cloud density, rain intensity, ...
            // doing min == max resolves to a constant target
            weather->m_SeverityMin = g_overrides.cloudDensity;
            weather->m_SeverityMax = g_overrides.cloudDensity;
            weather->m_SeverityTransitionTime = g_overrides.transitionTime;

            // how much of the downpour should be snow (snow starts at >0.5)
            weather->m_SnowRatio = g_overrides.snowRatio;

            weather->m_MinimumSnowAmount = g_overrides.groundSnow;
            weather->m_MaximumSnowAmount = g_overrides.groundSnow;

            weather->m_SnowRandomness = g_overrides.snowRandomness;

            weather->m_MinimumWetness = g_overrides.groundWetness;
            weather->m_MaximumWetness = g_overrides.groundWetness;

            weather->m_LightningPossibility = g_overrides.lightningChance;

            weather->m_CloudBase = g_overrides.cloudHeight;
            weather->m_CloudHeight = g_overrides.cloudThickness;

            weather->m_WindData.m_GustMapStrengthMin = g_overrides.gustStrength;
            weather->m_WindData.m_GustMapStrengthMax = g_overrides.gustStrength;

            // set m_UserControlled to stop automatic updates & allow custom values
            if (auto* precipitation = weather->m_Precipitation)
            {
                if (g_overrides.overrideDownpour)
                {
                    precipitation->m_UserControlled = true;
                    precipitation->m_Intensity = g_overrides.downpour;
                    precipitation->m_SnowRatio = g_overrides.snowRatio;
                    precipitation->m_ConstantAmount = 0.0f;
                    g_precipOwned = true;
                }
                else if (g_precipOwned)
                {
                    precipitation->m_UserControlled = false;
                    g_precipOwned = false;
                }
            }
        }

        inline void __fastcall HookedPreUpdate(CWeather* weather, bool paused, float deltaTime, void* camera)
        {
            ApplyOverrides(weather);

            if (g_originalPreUpdate)
                g_originalPreUpdate(weather, paused, deltaTime, camera);
        }

        inline void __fastcall HookedUpdate(CWeather* weather, float param1, bool param2, float param3, void* param4, bool param5)
        {
            ApplyOverrides(weather);

            if (g_originalUpdate)
                g_originalUpdate(weather, param1, param2, param3, param4, param5);
        }

        inline bool SetupWeatherHooks()
        {
            bool success = true;

            if (!g_originalPreUpdate)
                success &= MH_CreateHookGZ(WEATHER_PRE_UPDATE, &HookedPreUpdate, &g_originalPreUpdate);

            if (!g_originalUpdate)
                success &= MH_CreateHookGZ(WEATHER_UPDATE, &HookedUpdate, &g_originalUpdate);

            return success;
        }
    }
} // namespace gz
#pragma pack(pop)
