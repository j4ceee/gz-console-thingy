#pragma once

#pragma pack(push, 1)
namespace gz
{
    class CSkillManager
    {
    public:
        char pad_0x0[0x60]; // 0x000 → 0x060 (unknown)
    };
    static_assert(sizeof(CSkillManager) == 0x60, "CSkillManager size mismatch");

    class CPlayerSkills
    {
    };
} // namespace gz
#pragma pack(pop)
