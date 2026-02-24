#pragma once

#include "addresses.h"
#include "player_skills.h"
#include "save_game.h"

#pragma pack(push, 1)
namespace gz
{
    struct SXPRange
    {
        char        pad_0x00[0x18];     // 0x000 → 0x018 (unknown, possibly name hash or flags)
        int32_t     start_level;        // 0x018 → 0x01C
        uint32_t    end_level;          // 0x01C → 0x020
        int32_t     xp_per_level;       // 0x020 → 0x024
        char        pad_0x24[0x4];      // 0x024 → 0x028
    };
    static_assert(sizeof(SXPRange) == 0x28, "SXPRange size mismatch");

    class CPlayerProgressionManager
    {
    public:
        char        pad_0x000[0x90];        // 0x000 → 0x090 (unknown)
        SXPRange*   xp_ranges_begin;        // 0x090 → 0x098
        SXPRange*   xp_ranges_end;          // 0x098 → 0x0A0
        char        pad_0x0A0[0x60];        // 0x0A0 → 0x100
        int32_t     prestige_points;        // 0x100 → 0x104
        char        pad_0x104[0x1C];        // 0x104 → 0x120
        int32_t     character_level_cap;    // 0x120 → 0x124
        char        pad_0x124[0xDC];        // 0x124 → 0x200

        int32_t GetLevelCap() const
        {
            return character_level_cap;
        }

        int32_t GetXPAtLevel(uint32_t level) const
        {
            SXPRange* it = xp_ranges_begin;
            int32_t total = 0;
            if (it == xp_ranges_end) return 0;
            while (true) {
                if (level <= it->end_level) {
                    return total + ((int32_t)(level - it->start_level) + 1) * it->xp_per_level;
                }
                total += ((int32_t)(it->end_level - it->start_level) + 1) * it->xp_per_level;
                ++it;
                if (it == xp_ranges_end) return total;
            }
        }

        int32_t GetXPAtNextLevel(uint32_t current_level) const
        {
            return GetXPAtLevel(current_level + 1);
        }

        [[nodiscard]] int32_t GetPrestigePoints() const
        {
            return prestige_points;
        }

        void SetPrestigePoints(int32_t value)
        {
            prestige_points = value;
        }
    };

    class CPlayerInformation
    {
    public:
        char                        pad_0x0[0x20];          // 0x000 → 0x020
        CSkillManager               m_SkillManager;         // 0x020 → 0x080
        CLoginSaveLoader*           m_SaveLoader;           // 0x080 → 0x088
        char                        pad_0x88[0x8];          // 0x088 → 0x090
        CPlayerProgressionManager   m_ProgressionManager;   // 0x090 → 0x190

        static CPlayerInformation* instance()
        {
            return *reinterpret_cast<CPlayerInformation**>(GetAddress(INST_PLAYER_INFORMATION));
        }

        [[nodiscard]] CLoginSaveLoader* GetSaveLoader() const
        {
            return m_SaveLoader;
        }

        CPlayerProgressionManager* GetProgressionManager()
        {
            return &m_ProgressionManager;
        }
    };
} // namespace gz
#pragma pack(pop)