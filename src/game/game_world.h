#pragma once

#include "addresses.h"
#include "player.h"
#include "player_manager.h"
#include "../vector.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
class CGameWorld
{
public:
    static CGameWorld* instance()
    {
        auto* ptrToOffsetInstance = (uintptr_t*)GetAddress(INST_GAME_WORLD);
        uintptr_t offsetInstance = *ptrToOffsetInstance;
        return (CGameWorld*)(offsetInstance - 0xc0);
    }

    void TeleportToPositionXYZ(float x, float y, float z)
    {
        CVector3f targetPos = { x, y, z };
        SafeTeleportToPosition(targetPos, nullptr);
    }

    void TeleportToAimPosition(CPlayer* player = nullptr)
    {
        if (!player) {
            player = CNetworkPlayerManager::GetLocalPlayer();
        }
        if (!player) {
            return;
        }

        CVector3f aimPos = player->GetAimPosition();

        SafeTeleportToPosition(aimPos, player);
    }

    void SafeTeleportToPosition(CVector3f targetPos, CPlayer* player = nullptr)
    {
        if (!player) {
            player = CNetworkPlayerManager::GetLocalPlayer();
        }
        if (!player) {
            return;
        }
        CMatrix4f worldTransform = player->GetTransform();
        CMatrix4f worldTransformCurr = worldTransform;

        // only set position & keep current rotation/scale
        worldTransform.m[3].x = targetPos.x;
        worldTransform.m[3].y = targetPos.y;
        worldTransform.m[3].z = targetPos.z;

        auto distance = GetDistanceBetweenMatrixes(worldTransform, worldTransformCurr);

        // teleport
        if (distance < 700.0f)
        {
            TeleportPlayer(&worldTransform);
        }
        else
        {
            FastTravelToLocation(&worldTransform);
        }
    }

    // -- SIMPLE TELEPORTATION --

    // Virtual function at vtable index [26]
    // Based on JC4: TeleportPlayer(CMatrix4f* world, ...)
    void TeleportPlayer(CMatrix4f* worldTransform, void* param2 = nullptr)
    {
        meow_hook::func_call<void>(
            GetAddress(TELEPORT),
            this,
            worldTransform,
            param2
        );
    }

    // -- FAST TRAVEL --

    void FastTravelToLocation(CMatrix4f* worldTransform)
    {
        void* teleportObject = (void*)((uintptr_t)this + 0x10); // DAT_142abdaf8

        meow_hook::func_call<void>(
            GetAddress(FAST_TRAVEL),
            teleportObject,                 // param_1: manager
            worldTransform,                 // param_2: matrix
            nullptr,
            (char)0,
            (uint8_t)1,
            0.1f,
            0.1f,
            (uint32_t)0,
            nullptr,
            (uint8_t)0
        );
    }

    // -- DISTANCE CALCULATION --

    /// <summary>
    /// Calculates the distance between the translation components of two 4x4 matrices.
    /// </summary>
    /// <param name="a">The first matrix.</param>
    /// <param name="b">The second matrix.</param>
    /// <returns>The distance between the translation components of the two matrices.</returns>
    static float GetDistanceBetweenMatrixes(const CMatrix4f& a, const CMatrix4f& b)
    {
        float dx = a.m[3].x - b.m[3].x;
        float dy = a.m[3].y - b.m[3].y;
        float dz = a.m[3].z - b.m[3].z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
};
}
#pragma pack(pop)