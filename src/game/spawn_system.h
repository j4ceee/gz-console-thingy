#pragma once

#include <functional>
#include <memory>
#include <Windows.h>

#include "game_object.h"
#include "log.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
class CGameObject;
using spawned_objects = std::vector<std::shared_ptr<CGameObject>>;

class CSpawnSystem
{
  public:
    struct SResourceDef {
        char _pad[0x34]; // estimated
    };

    struct FakeVector {
        void* first;   // pointer to start of data
        void* last;    // pointer to current end
        void* end;     // pointer to allocated end
    };

    static CSpawnSystem* instance()
    {
        return *(CSpawnSystem**)GetAddress(INST_SPAWN_SYSTEM);
    }

    using success_t = void(__cdecl*)(const spawned_objects&, void*);
    using status_t  = void(__cdecl*)(int32_t, void*);

    struct SpawnReq {
        std::function<void(const spawned_objects&, void*)> callback;
        void*                                              userdata;
    };

    void SpawnCategoryAtAimPosition(const char* tag, char category)
    {
        switch (category)
        {
        case 'v':
            {
                const uint32_t VEHICLE_TYPE_ID = 0x1100FC;
                auto ownership_callback = (success_t)GetAddress(SPAWN_VEHICLE_OWNERSHIP_CALLBACK);
                SpawnTagAtAimPosition(tag, VEHICLE_TYPE_ID, ownership_callback);
                break;
            }
        case 'm':
        case 'n':
        default:
            {
                SpawnTagAtAimPosition(tag, 0x37C, nullptr);
                break;
            }
        }
    }

    void SpawnTagAtAimPosition(const char* tag, uint32_t type_id = 0x37C, success_t callback = nullptr)
    {
        CPlayer* player = CNetworkPlayerManager::GetLocalPlayer();
        CVector3f aimPos = player->GetAimPosition();
        CMatrix4f worldTransform = player->GetTransform();
        worldTransform.m[3].x = aimPos.x;
        worldTransform.m[3].y = aimPos.y;
        worldTransform.m[3].z = aimPos.z;
        SpawnByTag(tag, type_id, worldTransform, callback);
    }

    void SpawnTagAtAimPosition(char* tag, uint32_t type_id = 0x37C, success_t callback = nullptr)
    {
        SpawnTagAtAimPosition(const_cast<const char*>(tag), type_id, callback);
    }

    void SpawnHashAtAimPosition(uint32_t hash, uint32_t type_id = 0x37C, success_t callback = nullptr)
    {
        CPlayer* player = CNetworkPlayerManager::GetLocalPlayer();
        CVector3f aimPos = player->GetAimPosition();
        CMatrix4f worldTransform = player->GetTransform();
        // only set position & keep current rotation/scale
        worldTransform.m[3].x = aimPos.x;
        worldTransform.m[3].y = aimPos.y;
        worldTransform.m[3].z = aimPos.z;
        SpawnByHash(hash, type_id, worldTransform, callback);
    }

    void SpawnByTag(const char* tag, uint32_t type_id, const CMatrix4f& transform, success_t callback = nullptr)
    {
        int32_t tags[32] = {};
        int32_t invalid = 0;
        ParseTags(tag, tags, &invalid);

        SpawnByHash(tags[0], type_id, transform, callback);
    }

    void SpawnByHash(int32_t hash, uint32_t type_id, const CMatrix4f& transform, success_t callback = nullptr)
    {
        int32_t tags[32] = {};
        tags[0] = hash;
        std::vector<SResourceDef*> resources;
        GetMatchingResources(tags, &resources, false);

        if (resources.empty()) {
            Log("[SPAWN] No resources found for hash 0x%X", hash);
            return;
        }

        auto* data = (uint32_t*)resources[0];
        uint32_t spawn_id = data[2];

        CMatrix4f spawn_matrix = transform;

        // dummy arg for param_9 (must be valid memory)
        uint64_t dummy_val = 0;

        success_t callback_ptr = callback ? callback : &CSpawnSystem::OnSpawnCompleteStatic;


        meow_hook::func_call<void>(
            GetAddress(SPAWN_SYSTEM_SPAWN),
            this,
            spawn_id,           // RDX: calculated id
            &spawn_matrix,      // R8: matrix pointer
            type_id,            // R9: not sure, flag?
            callback_ptr,       // Stack 1: callback after spawn
            nullptr,            // Stack 2: userdata
            0xFFFFFFFF,         // Stack 3: flags
            nullptr,            // Stack 4: unused
            &dummy_val          // Stack 5: valid pointer
        );
    }

    static void __cdecl OnSpawnCompleteStatic(const spawned_objects& objects, void* userdata)
    {
        Log("[CALLBACK] Spawn completed! %zu objects created", objects.size());
    }

    // Parse model tags (like "skir_classa_load01") into hashes
    int ParseTags(const char* search_string, int32_t* out_tags, int32_t* out_invalid_count) {
        return meow_hook::func_call<int>(
            GetAddress(SPAWN_SYSTEM_PARSE_TAGS),
            this,
            search_string,
            out_tags,
            out_invalid_count
        );
    }

    // Get resources matching parsed tags
    bool GetMatchingResources(int32_t* tags, std::vector<SResourceDef*>* out_resources, bool shuffle = false)
    {
        SResourceDef* ptr_buffer[4096];

        FakeVector safe_vec{};
        safe_vec.first = ptr_buffer;
        safe_vec.last  = ptr_buffer; // starts empty
        safe_vec.end   = ptr_buffer + 4096; // capacity limit

        bool success = false;

        meow_hook::func_call<void>(
            GetAddress(SPAWN_SYSTEM_GET_MATCHING_RES),
            this,
            tags,
            &safe_vec,      // custom vector for safety
            (char)0,        // unknown
            (char)0,        // unknown
            (char)shuffle,
            &success
        );

        // copy the results back to real std::vector
        // calculate how many items the game wrote by checking how far last moved
        size_t count = ((uintptr_t)safe_vec.last - (uintptr_t)safe_vec.first) / sizeof(void*);

        if (count > 0 && count <= 4096) {
            out_resources->reserve(count);
            for (size_t i = 0; i < count; i++) {
                out_resources->push_back(ptr_buffer[i]);
            }
        }

        // the function sets the bool to true if it processed successfully
        return success && !out_resources->empty();
    }

public:
    char                _pad[0x48];           // +0x00 → +0x48
    void*               m_tagBitmasks;        // +0x48: tag bitmask pointer
    char                _pad2[0x10];          // +0x50 → +0x60 (16 bytes)
    void*               m_stringTree;         // +0x60: string lookup tree
    uint32_t            m_resourceCount;      // +0x68: resource count (5227)
    char                _pad3[4];             // +0x6C → +0x70
    uint32_t            m_bitmaskWordCount;   // +0x70: bitmask word count pointer
    char                _pad4[4];             // +0x74 → +0x78
    SResourceDef*       m_resourceDefs;       // +0x78: resource definitions array pointer
    char                _pad5[0xA4];          // +0x80 → +0x124
    float               m_playerPosZ;         // +0x124
    float               m_playerPosX;         // +0x128
    float               m_playerPosY;         // +0x12C
    char                _pad6[0x10];          // +0x130 → +0x140
    CRITICAL_SECTION    m_criticalSection;    // +0x140
};

    static_assert(offsetof(CSpawnSystem, m_tagBitmasks) == 0x48, "Offset mismatch: m_tagBitmasks");
    static_assert(offsetof(CSpawnSystem, m_stringTree) == 0x60, "Offset mismatch: m_stringTree");
    static_assert(offsetof(CSpawnSystem, m_resourceCount) == 0x68, "Offset mismatch: m_resourceCount");
    static_assert(offsetof(CSpawnSystem, m_bitmaskWordCount) == 0x70, "Offset mismatch: m_bitmaskWordCount");
    static_assert(offsetof(CSpawnSystem, m_resourceDefs) == 0x78, "Offset mismatch: m_resourceDefs");

} // namespace gz
#pragma pack(pop)