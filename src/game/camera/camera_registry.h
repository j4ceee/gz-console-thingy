#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "../../addresses.h"
#include "../../log.h"
#include "../util/hash_utils.h"
#include "camera_director.h"

#pragma pack(push, 1)
namespace gz
{
    struct SCameraChain
    {
        SCameraId m_Key; // 0x00
        uint16_t m_Next; // 0x08  0xFFFF = end of chain
        uint8_t _pad[6]; // 0x0A
        void* m_Value; // 0x10  CCameraPipeline*
    };
    static_assert(sizeof(SCameraChain) == 0x18);


    struct SCameraHashTable
    {
        uint16_t*       m_HashTable;        // 0x00  bucket array, m_HashTableLength entries
        SCameraChain*   m_ChainPool;        // 0x08
        uint16_t        m_HashTableLength;  // 0x10
        uint16_t        m_FirstFreeChain;   // 0x12
        uint16_t        m_ChainPoolSize;    // 0x14  capacity
        uint16_t        m_ChainPoolFreeCount; // 0x16
        uint16_t        m_Size;             // 0x18  live entries
        uint16_t        m_Flags;            // 0x1A
        uint8_t         _pad[4];            // 0x1C
    };
    static_assert(sizeof(SCameraHashTable) == 0x20);


    class CCameraPipeline
    {
    public:
        uint8_t     _pad0[0x118];       // 0x000
        void*       m_Object;           // 0x118 CCameraObject*
        uint8_t     _pad1[0x60];        // 0x120
        SCameraId   m_Id;               // 0x180
        SCameraId   m_ReferenceNameId;  // 0x188
        uint8_t     _pad2[0x10];        // 0x190
        uint64_t    m_TransitionGroups; // 0x1A0
        uint8_t     _pad3[0x460];       // 0x1A8
        bool        m_DeactivateIfNotTop; // 0x608
        bool        m_UpdateDuringPause; // 0x609
        bool        m_IsActive;         // 0x60A
        bool        m_BlockControls;    // 0x60B
        int32_t     m_Prio;             // 0x60C
    };
    static_assert(offsetof(CCameraPipeline, m_Id) == 0x180);
    static_assert(offsetof(CCameraPipeline, m_Prio) == 0x60C);


    class CCameraObject
    {
    public:
        uint8_t _pad0[0x80]; // 0x00
        SCameraId m_ObjectID; // 0x80
        SCameraId m_EntityObjectID; // 0x88 shared by all objects of one entity
        uint32_t m_NameHash; // 0x90
    };
    static_assert(offsetof(CCameraObject, m_NameHash) == 0x90);


    struct SCameraEntry
    {
        uint32_t m_NameHash;
        SCameraId m_Id;
        CCameraPipeline* m_Pipeline;
        SCameraId m_EntityId;
    };


    class CCameraRegistry
    {
    public:
        uint8_t             _pad0[0xA0];            // 0x000
        SCameraHashTable    m_RegisteredPipelines;  // 0x0A0
        uint8_t             _pad1[0x58];            // 0x0C0
        void*               m_Mutex;                // 0x118

        static CCameraRegistry* instance()
        {
            return *(CCameraRegistry**)(GetAddress(INST_CAMERA_REGISTRY));
        }

        /// Walks every chain and returns one entry per registered camera
        [[nodiscard]] std::vector<SCameraEntry> Enumerate() const
        {
            std::vector<SCameraEntry> out;

            const auto& t = m_RegisteredPipelines;
            if (!t.m_HashTable || !t.m_ChainPool || t.m_HashTableLength == 0)
                return out;

            out.reserve(t.m_Size);

            for (uint32_t bucket = 0; bucket < t.m_HashTableLength; ++bucket)
            {
                uint16_t idx = t.m_HashTable[bucket];
                // bound the walk by the pool size - a corrupt m_Next can never hang us
                for (uint32_t guard = 0; idx != 0xFFFF && guard <= t.m_ChainPoolSize; ++guard)
                {
                    if (idx >= t.m_ChainPoolSize) break;

                    const SCameraChain& chain = t.m_ChainPool[idx];
                    auto* pipeline = static_cast<CCameraPipeline*>(chain.m_Value);

                    if (pipeline)
                    {
                        auto* obj = static_cast<CCameraObject*>(pipeline->m_Object);
                        out.push_back({
                            obj ? obj->m_NameHash : 0u,
                            chain.m_Key,
                            pipeline,
                            obj ? obj->m_EntityObjectID : SCameraId{}
                        });
                    }
                    idx = chain.m_Next;
                }
            }
            return out;
        }

        /// Finds a registered camera by its name hash.
        /// Logs when more than one camera shares the hash (e.g. SharedThirdPersonCamera).
        [[nodiscard]] bool FindByName(uint32_t nameHash, SCameraEntry* out) const
        {
            const auto entries = Enumerate();
            const SCameraEntry* found = nullptr;
            int matches = 0;

            for (const auto& e : entries)
            {
                if (e.m_NameHash != nameHash) continue;
                ++matches;
                if (!found) found = &e;
            }

            if (matches > 1)
                Log("CameraRegistry: name hash 0x%08X is ambiguous (%d matches), using the first",
                    nameHash, matches);

            if (!found) return false;
            *out = *found;
            return true;
        }
    };

    static_assert(offsetof(CCameraRegistry, m_RegisteredPipelines) == 0xA0);
    static_assert(offsetof(CCameraRegistry, m_Mutex) == 0x118);
} // namespace gz
#pragma pack(pop)

namespace gz::CameraNames
{
    /// all CCameraObjects from local_player_character.epe.
    /// m_NameHash is HashBuffer(name, 0)
    inline constexpr const char* kKnownNames[] = {
        "FirstPersonStandWP",
        "FirstPersonCrouchWP",
        "FirstPersonJogWP",
        "FirstPersonSprintWP",
        "FirstPersonProneWP",
        "IronSightStand",
        "IronSightCrouch",
        "IronSightProne",
        "AimScopeStand",
        "AimScopeCrouch",
        "AimScopeProne",
        "BinocularAimScopeStand",
        "BinocularAimScopeCrouch",
        "BinocularAimScopeProne",
        "SharedThirdPersonCamera",
        "DebugThirdPersonCamera",
        "MachineThirdPersonCamera",
        "SharedThirdPersonBikeCamera",
        "SharedThirdPersonVehicleCamera",
        "VehicleDriving",
        "DownedCamera",
        "RagdollingCamera",
        "GauntletRatCamera",
    };

    inline const char* Lookup(uint32_t nameHash)
    {
        static std::unordered_map<uint32_t, const char*> s_map;
        if (s_map.empty())
            for (const char* name : kKnownNames)
                s_map.emplace(Utils::HashBuffer(name, 0), name);

        const auto it = s_map.find(nameHash);
        return it != s_map.end() ? it->second : "<unknown>";
    }
}
