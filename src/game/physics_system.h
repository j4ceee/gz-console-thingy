#pragma once

#include "addresses.h"

#pragma pack(push, 1)
namespace gz
{
    // Havok physics world (hknpWorld)
    class hknpWorld
    {
    public:
        char pad_0x0[0x974];            // 0x000 → 0x974
        float gravity;                  // 0x974 → 0x978

        float GetGravity()
        {
            return gravity;
        }

        void SetGravity(float value)
        {
            gravity = value;
        }

        float GetGravityInGs()
        {
            return gravity / -9.810f;
        }

        void SetGravityInGs(float gs)
        {
            gravity = gs * -9.810f;
        }
    };

    // physics system
    class CPhysicsSystem
    {
    public:
        char pad_0x0[0x38];             // 0x000 → 0x038
        hknpWorld* world;               // 0x038 → 0x040 (pointer to hknpWorld)

        static CPhysicsSystem* instance()
        {
            return *(CPhysicsSystem**)(GetAddress(INST_PHYSICS_SYSTEM));
        }

        hknpWorld* GetWorld()
        {
            return world;
        }
    };
}; // namespace gz
#pragma pack(pop)