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

    inline float g_thirdPersonCameraDistance = 1.0f;

    class CCameraCollisionModifier
    {
    public:
        static void SetThirdPersonCameraDistance(float distance)
        {
            g_thirdPersonCameraDistance = distance;
        }

        static float GetThirdPersonCameraDistance()
        {
            return g_thirdPersonCameraDistance;
        }

        static void ChangeThirdPersonCameraDistance(float delta)
        {
            float newDistance = g_thirdPersonCameraDistance + delta;
            if (newDistance < 0.0f)
            {
                g_thirdPersonCameraDistance = 0.0f;
            }
            else if (newDistance > 1.0f)
            {
                g_thirdPersonCameraDistance = 1.0f;
            }
            else
            {
                g_thirdPersonCameraDistance = newDistance;
            }
        }

        static void CheckCollisionHook(CCameraCollisionModifier* modifier, SCameraCollisionInfo* info, CMatrix4f* matrix, CVector3f* vector, float param4)
        {
            if (g_checkCollision == nullptr) {
                return;
            }
            g_checkCollision(modifier, info, matrix, vector, param4);

            auto* character = CNetworkPlayerManager::instance()->GetPlayer()->GetCharacter();
            if (character->IsControllingEntity())
            {
                info->m_Distance = g_thirdPersonCameraDistance; // set camera distance when controlling RC
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