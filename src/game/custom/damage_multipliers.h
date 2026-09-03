#pragma once

#include "addresses.h"
#include "hook_helpers.h"
#include "game/animal_character.h"
#include "game/player_skills.h"

namespace gz::DamageMultipliers
{
    inline float g_applyBulletDamageMultiplier = 1.0f;

    using ApplyBulletSkillDamageMultiplierFunc = void(*)(CAnimalCharacterComponent* cc, CPlayerSkills* skills, void* msg, float* dmg);
    inline ApplyBulletSkillDamageMultiplierFunc g_applyBulletDamageMult = nullptr;

    inline void HookedApplyBulletSkillDamageMultiplier(CAnimalCharacterComponent* cc, CPlayerSkills* skills, void* msg, float* dmg)
    {
        g_applyBulletDamageMult(cc, skills, msg, dmg);
        if (dmg) *dmg *= g_applyBulletDamageMultiplier;
    }

    inline bool SetupBulletSkillDmgMultHook()
    {
        if (g_applyBulletDamageMult != nullptr) return true; // already set up
        return MH_CreateHookGZ(ANIMAL_APPLY_BULLET_SKILL_DAMAGE, &HookedApplyBulletSkillDamageMultiplier, &g_applyBulletDamageMult);
    }
} // namespace gz::DamageMultipliers
