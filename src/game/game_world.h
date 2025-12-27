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

    void TeleportToPositionXYZ(float x, float y, float z)
    {
        CMatrix4f worldTransform;
        worldTransform.m[3].x = x;
        worldTransform.m[3].y = y;
        worldTransform.m[3].z = z;

        TeleportPlayer(&worldTransform, nullptr);
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
        CMatrix4f worldTransform = player->GetTransform();
        // only set position & keep current rotation/scale
        worldTransform.m[3].x = aimPos.x;
        worldTransform.m[3].y = aimPos.y;
        worldTransform.m[3].z = aimPos.z;
        // teleport
        TeleportPlayer(&worldTransform, nullptr);
    }
};
}
#pragma pack(pop)