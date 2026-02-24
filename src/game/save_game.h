#pragma once

#include <cstdint>

#pragma pack(push, 1)
namespace gz
{
    // individual player data structure (0x5D0 bytes per player)
    struct SPlayerData
    {
        char     pad_0x0[0x308];   // 0x000 → 0x308
        int32_t  skill_points;     // 0x308 → 0x30C
        char     pad_0x30C[0x24];  // 0x30C → 0x330
        int32_t  experience;       // 0x330 → 0x334
        uint16_t level;            // 0x334 → 0x336
        char     pad_0x336[0x2];   // 0x336 → 0x338
        void*    item_list;        // 0x338 → 0x340
        char     pad_0x340[0x290]; // 0x340 → 0x5D0

        int32_t GetSkillPoints()
        {
            if (skill_points < 0)
            {
                SetSkillPoints(0);
                return 0;
            }
            return skill_points;
        }

        int32_t GetExperience() const
        {
            return experience;
        }

        uint16_t GetLevel() const
        {
            return level;
        }

        void SetSkillPoints(int32_t points)
        {
            this->skill_points = points;
        }

        void SetExperience(int32_t exp)
        {
            this->experience = exp;
        }

        void SetLevel(uint16_t lvl)
        {
            this->level = lvl;
        }
    };
    static_assert(sizeof(SPlayerData) == 0x5D0, "PlayerData size mismatch");


    struct SWorldSaveSlot
    {
        char     pad_0x000[0x1A0];      // 0x000 → 0x1A0
        int32_t  command_tokens;        // 0x1A0 → 0x1A4
        char     pad_0x1A4[0x3C];       // 0x1A4 → 0x1E0

        int32_t GetCommandTokens() const
        {
            return command_tokens;
        }

        void SetCommandTokens(const int32_t tokens)
        {
            this->command_tokens = tokens;
        }
    };
    static_assert(sizeof(SWorldSaveSlot) == 0x1E0, "SWorldSaveSlot size mismatch");


    class CLoginSaveLoader
    {
    public:
        char                pad_0x0[0x20];      // 0x000 → 0x020
        void*               m_AdfContext;       // 0x020 → 0x028 (set to AdfContextNew() result)
        char                pad_0x28[0x28];     // 0x028 → 0x050
        SWorldSaveSlot      world_slots[4];     // 0x050 → 0x7D0 (array of 4 world save slots, 0x1E0 each = 0x780)
        uint32_t            active_world;       // 0x7D0 → 0x7D4
        char                pad_0x7D4[0xCC];    // 0x7D4 → 0x8A0
        void*               resource_list;      // 0x8A0 → 0x8A8 (pointer to resource collection)
        char                pad_0x8A8[0x10];    // 0x8A8 → 0x8B8
        void*               storage;            // 0x8B8 → 0x8C0 (pointer to storage)
        char                pad_0x8C0[0x1C0];   // 0x8C0 → 0xA80
        SPlayerData          players[4];         // 0xA80 → 0x21C0 (array of 4 players, 0x5D0 each = 0x1740)
        int32_t             m_ActivePlayer;     // 0x21C0 → 0x21C4 (index of active player, 0-3)
        char                pad_0x21C4[0x74];   // 0x21C4 → 0x2238

        int32_t GetActivePlayerIndex() const
        {
            return m_ActivePlayer;
        }

        SPlayerData* GetPlayerData(int player_index) const
        {
            if (player_index < 0)
            {
                player_index = 0;
            }
            else if (player_index > 3)
            {
                player_index = 3;
            }

            return const_cast<SPlayerData*>(&players[player_index]);
        }

        SPlayerData* GetActivePlayerData() const
        {
            return GetPlayerData(m_ActivePlayer);
        }

        uint32_t GetActiveWorldIndex() const
        {
            return active_world;
        }

        SWorldSaveSlot* GetWorldSaveSlot(int world_index) const
        {
            if (world_index < 0)
            {
                world_index = 0;
            }
            else if (world_index > 3)
            {
                world_index = 3;
            }

            return const_cast<SWorldSaveSlot*>(&world_slots[world_index]);
        }

        SWorldSaveSlot* GetActiveWorldSaveSlot() const
        {
            return GetWorldSaveSlot(active_world);
        }
    };
    static_assert(sizeof(CLoginSaveLoader) == 0x2238, "CSkillManager size mismatch");
} // namespace gz
#pragma pack(pop)
