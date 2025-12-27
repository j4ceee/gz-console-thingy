#pragma once

#include "addresses.h"

#pragma pack(push, 1)
namespace gz
{
    // World time / day-night cycle system
    class CWorldTime
    {
    public:
        char pad_0x0[0xE0];             // 0x000 → 0x0E0
        float m_time;                   // 0x0E0 → 0x0E4 (0-24 hours)
        float m_pauseState;             // 0x0E4 → 0x0E8
        float m_timeScale;              // 0x0E8 → 0x0EC (default 1.0)

        static CWorldTime* instance()
        {
            return *(CWorldTime**)(GetAddress(INST_WORLDTIME));
        }

        // ========== GETTERS ==========

        float GetTime()
        {
            return m_time;
        }

        float GetTimeScale()
        {
            return m_timeScale;
        }

        void GetTimeAsHHMM(int& outHours, int& outMinutes)
        {
            float safeTime = std::fmod(m_time, 24.0f);
            if (safeTime < 0.0f) safeTime += 24.0f;

            outHours = static_cast<int>(safeTime);
            float fractionalHour = safeTime - outHours;
            outMinutes = static_cast<int>(fractionalHour * 60.0f + 0.5f); // round to nearest minute

            // handle overflow (e.g., 14:60 → 15:00)
            if (outMinutes >= 60) {
                outMinutes = 0;
                outHours++;
                if (outHours >= 24) {
                    outHours = 0;
                }
            }
        }

        bool IsPaused()
        {
            return m_timeScale == 0.0f;
        }

        // ========== SETTERS ==========

        void SetTime(float value)
        {
            // matches engine logic: wraps time (25.0 -> 1.0) rather than clamping
            m_time = std::fmod(value, 24.0f);
            if (m_time < 0.0f) m_time += 24.0f;
        }

        void SetTimeScale(float value)
        {
            m_timeScale = value;
        }

        void SetPaused(bool state)
        {
            if (state) {
                m_timeScale = 0.0f;
            } else {
                if (m_timeScale == 0.0f) {
                    m_timeScale = 1.0f; // reset to normal speed if unpausing from paused state
                }
            }
        }

        void FreezeTime()
        {
            SetPaused(true);
        }

        void UnfreezeTime()
        {
            SetPaused(false);
        }

        void ResetTimeScale()
        {
            SetTimeScale(1.0f);
        }

        void SetTimeAsHHMM(int hours, int minutes)
        {
            float newTime = static_cast<float>(hours) + (static_cast<float>(minutes) / 60.0f);
            SetTime(newTime);
        }
    };
}; // namespace gz
#pragma pack(pop)