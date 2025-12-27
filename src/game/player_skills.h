#pragma once

#pragma pack(push, 1)

namespace gz
{
class CPlayerSkills
{
public:
    // actual object starts at pointer - 0x64
    char _pad[0x64];
};
} // namespace gz
#pragma pack(pop)
