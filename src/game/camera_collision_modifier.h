#pragma once

#include "addresses.h"
#include "player_network_manager.h"
#include "hook_helpers.h"

#pragma pack(push, 1)
namespace gz
{
    class CCameraCollisionModifier;

    struct SCameraCollisionInfo
    {
        char    _pad[0x1C];  // 0x00 → 0x1C
        float   m_Distance;  // 0x1C → 0x20
    };

    using CheckCollisionFunc = void(*)(CCameraCollisionModifier* modifier, SCameraCollisionInfo* info, CMatrix4f* matrix, CVector3f* vector, float param4);
    inline CheckCollisionFunc g_checkCollision = nullptr;

    class CCameraCollisionModifier
    {
    public:
        static void CheckCollisionHook(CCameraCollisionModifier* modifier, SCameraCollisionInfo* info, CMatrix4f* matrix, CVector3f* vector, float param4)
        {
            if (g_checkCollision == nullptr) {
                return;
            }
            g_checkCollision(modifier, info, matrix, vector, param4);

            auto* character = CNetworkPlayerManager::instance()->GetPlayer()->GetCharacter();
            auto* rc = character ? character->GetRemoteController() : nullptr;
            if (rc && rc->GetControlledCharacter())
            {
                info->m_Distance = 1.0f; // set to max distance
            }
        }
        static bool SetupCheckCollisionHook()
        {
            if (g_checkCollision != nullptr) {
                return true; // already set up
            }
            return MH_CreateHookGZ(CAMERA_CHECK_COLLISION, &CheckCollisionHook, &g_checkCollision);
        }
    };
} // namespace gz
#pragma pack(pop)