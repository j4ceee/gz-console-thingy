#pragma once

#include "addresses.h"

#pragma pack(push, 1)
namespace gz
{
    // individual player data structure (0x5D0 bytes per player)
    struct PlayerData
    {
        char pad_0x0[0x308];            // 0x000 → 0x308
        int32_t skill_points;           // 0x308 → 0x30C
        char pad_0x30C[0x24];           // 0x30C → 0x330
        int32_t experience;             // 0x330 → 0x334
        uint16_t level;                 // 0x334 → 0x336
        char pad_0x336[0x2];            // 0x336 → 0x338
        void* item_list;                // 0x338 → 0x340 (pointer to item collection)
        char pad_0x340[0x290];          // 0x340 → 0x5D0
    };

    // login save data structure
    struct CLoginSaveLoader
    {
        char pad_0x0[0x1F0];            // 0x000 → 0x1F0
        int32_t command_tokens;         // 0x1F0 → 0x1F4
        char pad_0x1F4[0x6AC];          // 0x1F4 → 0x8A0
        void* resource_list;            // 0x8A0 → 0x8A8 (pointer to resource collection)
        char pad_0x8A8[0x10];           // 0x8A8 → 0x8B8
        void* storage;                  // 0x8B8 → 0x8C0 (pointer to storage)
        char pad_0x8C0[0x1C0];          // 0x8C0 → 0xA80
        PlayerData players[4];          // 0xA80 → 0x21C0 (array of 4 players, 0x5D0 each = 0x1740)
        int32_t active_player;          // 0x21C0 (0-3 for player index)
    };

    class CPlayerInformation
    {
    public:
        char pad_0x0[0x80];             // 0x000 → 0x080
        CLoginSaveLoader* save_loader;  // 0x080 → 0x088 (pointer to save loader)
        char pad_0x88[0x108];           // 0x088 → 0x190
        int32_t prestige_points;        // 0x190 → 0x194 (DIRECT value)

        static CPlayerInformation* instance()
        {
            return *(CPlayerInformation**)(GetAddress(INST_PLAYER_INFORMATION));
        }

        // ========== GETTERS ==========

        CLoginSaveLoader* GetSaveLoader()
        {
            return save_loader;
        }

        int32_t GetActivePlayer()
        {
            if (!save_loader) return -1;
            return save_loader->active_player;
        }

        PlayerData* GetPlayerData(int player_index)
        {
            if (player_index < 0 || player_index > 3) return nullptr;
            if (!save_loader) return nullptr;
            return &save_loader->players[player_index];
        }

        PlayerData* GetActivePlayerData()
        {
            int activeIndex = GetActivePlayer();
            return GetPlayerData(activeIndex);
        }

        int32_t GetCommandTokens()
        {
            if (!save_loader) return -1;
            return save_loader->command_tokens;
        }

        int32_t GetPrestigePoints()
        {
            return prestige_points;
        }

        int32_t GetSkillPoints(int player_index)
        {
            PlayerData* playerData = GetPlayerData(player_index);
            if (!playerData) return -1;
            return playerData->skill_points;
        }

        int32_t GetSkillPointsForActive()
        {
            int activeIndex = GetActivePlayer();
            return GetSkillPoints(activeIndex);
        }

        int32_t GetExperience(int player_index)
        {
            PlayerData* playerData = GetPlayerData(player_index);
            if (!playerData) return -1;
            return playerData->experience;
        }

        int32_t GetExperienceForActive()
        {
            int activeIndex = GetActivePlayer();
            return GetExperience(activeIndex);
        }

        uint16_t GetLevel(int player_index)
        {
            PlayerData* playerData = GetPlayerData(player_index);
            if (!playerData) return 0;
            return playerData->level;
        }

        uint16_t GetLevelForActive()
        {
            int activeIndex = GetActivePlayer();
            return GetLevel(activeIndex);
        }

        // ========== SETTERS ==========

        // set skill points for specific player (0-3)
        bool SetSkillPoints(int player_index, int32_t value)
        {
            PlayerData* playerData = GetPlayerData(player_index);
            if (!playerData) return false;
            playerData->skill_points = value;
            return true;
        }

        // set skill points for active player
        bool SetSkillPointsForActive(int32_t value)
        {
            int activeIndex = GetActivePlayer();
            return SetSkillPoints(activeIndex, value);
        }

        // set experience for specific player (0-3)
        bool SetExperience(int player_index, int32_t value)
        {
            PlayerData* playerData = GetPlayerData(player_index);
            if (!playerData) return false;
            playerData->experience = value;
            return true;
        }

        // set experience for active player
        bool SetExperienceForActive(int32_t value)
        {
            int activeIndex = GetActivePlayer();
            return SetExperience(activeIndex, value);
        }

        // set level for specific player (0-3)
        bool SetLevel(int player_index, uint16_t value)
        {
            PlayerData* playerData = GetPlayerData(player_index);
            if (!playerData) return false;
            playerData->level = value;
            return true;
        }

        // set level for active player
        bool SetLevelForActive(uint16_t value)
        {
            int activeIndex = GetActivePlayer();
            return SetLevel(activeIndex, value);
        }

        // set command tokens
        bool SetCommandTokens(int32_t value)
        {
            if (!save_loader) return false;
            save_loader->command_tokens = value;
            return true;
        }

        // set prestige points
        bool SetPrestigePoints(int32_t value)
        {
            prestige_points = value;
            return true;
        }
    };
}; // namespace gz
#pragma pack(pop)