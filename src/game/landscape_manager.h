#pragma once

#include <cstddef>
#include "addresses.h"
#include "weather.h"

#pragma pack(push, 1)
namespace gz
{
    class CLandscapeManager
    {
    public:
        char _pad0[0x188]; // +0x000
        CAtmosphere* m_Atmosphere; // +0x188

        static CLandscapeManager* instance()
        {
            return *(CLandscapeManager**)(GetAddress(INST_LANDSCAPE_MANAGER));
        }

        [[nodiscard]] CAtmosphere* GetAtmosphere() const
        {
            return m_Atmosphere;
        }

        [[nodiscard]] CWeather* GetWeather() const
        {
            if (!m_Atmosphere || !m_Atmosphere->IsInitialized()) return nullptr;
            return m_Atmosphere->GetWeather();
        }
    };
    static_assert(offsetof(CLandscapeManager, m_Atmosphere) == 0x188);
} // namespace gz
#pragma pack(pop)
