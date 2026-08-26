#pragma once

#include "data_types.h"
#include "hook_helpers.h"
#include "log.h"
#include "meow_hook/util.h"
#include <string>

#pragma pack(push, 1)
namespace gz
{
    struct SResourceInstanceSet
    {
        BasicVector Resources; // 0x00
        BasicVector m_Handles; // 0x18
    };
    static_assert(sizeof(SResourceInstanceSet) == 0x30);

    class CEntityResourceLoader
    {
    public:
        char _pad0[0x18];
        SResourceInstanceSet InstanceSet;   // 0x18
    };
    static_assert(offsetof(CEntityResourceLoader, InstanceSet) == 0x18);

    class CPlayerSpawnManager
    {
    public:
        BasicVector             m_EntityNames;              // 0x00 → 0x18 [0]=local, [1]=remote
        CEntityResourceLoader*  m_EntityResourceLoader;     // 0x18 → 0x20
        char                    _pad0[0x20];                // 0x20 → 0x40
        bool                    m_ReqStopLoading;           // 0x40 → 0x41
        bool                    m_ReqStopLoadingComplete;   // 0x41 → 0x42
        char                    _pad1[0x76];                // 0x20 → 0x40
        SResourceInstanceSet    m_ResourceSet;              // 0xB8

        static CPlayerSpawnManager* instance()
        {
            return *reinterpret_cast<CPlayerSpawnManager**>(GetAddress(INST_PLAYER_SPAWN_MANAGER));
        }

        [[nodiscard]] bool IsLoading() const
        {
            return m_EntityResourceLoader != nullptr;
        }

        [[nodiscard]] GameString* GetEntityName(int index) const
        {
            auto* b = static_cast<uint8_t*>(m_EntityNames.first);
            auto* e = static_cast<uint8_t*>(m_EntityNames.last);
            if (!b || index < 0 || index >= static_cast<int>((e - b) / sizeof(GameString))) return nullptr;
            return reinterpret_cast<GameString*>(b + index * sizeof(GameString));
        }

        // peerId < 0xFE -> RemotePlayerExpentity (tp_player_character.expentity)
        // peerId >= 0xFE -> LocalPlayerExpentity (local_player_character.expentity)
        bool SpawnPlayer(uint8_t peerId, const CMatrix4f& matrix, uint32_t m_SpawningNetworkId = 0)
        {
            return meow_hook::func_call<bool>(GetAddress(SPAWN_PLAYER), this, peerId, &matrix, m_SpawningNetworkId);
        }
    };
    static_assert(offsetof(CPlayerSpawnManager, m_EntityNames) == 0x00);
    static_assert(offsetof(CPlayerSpawnManager, m_EntityResourceLoader) == 0x18);
    static_assert(offsetof(CPlayerSpawnManager, m_ReqStopLoading) == 0x40);
    static_assert(offsetof(CPlayerSpawnManager, m_ResourceSet) == 0xB8);

    // --- Third-person resource priming ---
    // Spawns throwaway tp_player_character entity to trigger archive-mount pipeline (CEntityProvider::Update -> LoadResources)
    // No character is ever actually constructed

    using EntityLoadedFunc = void(__fastcall*)(CPlayerSpawnManager*);
    inline EntityLoadedFunc g_originalEntityLoaded = nullptr;
    inline bool g_primingThirdPersonResources = false;
    inline bool g_thirdPersonResourcesPrimed = false;

    /// Primes the resource cache with tp_local_player_character.ee (NOT tp_player_character.ee, it lacks the two player_thirdperson_control layers)
    /// Temporarily rewrites the remote entity name so the game's own load pipeline fetches the local-TP archive;
    inline bool PrimeThirdPersonResources()
    {
        auto* mgr = CPlayerSpawnManager::instance();
        if (!mgr || mgr->IsLoading()) return false;

        GameString* remote = mgr->GetEntityName(1);
        if (!remote)
        {
            Log("PrimeThirdPersonResources: no remote entity name");
            return false;
        }

        std::string name = remote->c_str();
        Log("PrimeThirdPersonResources: remote entity is '%s'", name.c_str());

        constexpr const char* kFrom = "tp_player_character";
        constexpr const char* kTo = "tp_local_player_character";
        const size_t pos = name.find(kFrom);
        if (pos == std::string::npos)
        {
            Log("PrimeThirdPersonResources: unexpected remote entity name, aborting");
            return false;
        }
        name.replace(pos, strlen(kFrom), kTo);

        uint8_t saved[sizeof(GameString)];
        memcpy(saved, remote, sizeof(GameString));
        remote->SetTemporary(name.c_str());

        g_primingThirdPersonResources = true;
        CMatrix4f identity{};
        const bool ok = mgr->SpawnPlayer(0xF0, identity);

        remote->FreeTemporary();
        memcpy(remote, saved, sizeof(GameString));

        if (!ok)
        {
            g_primingThirdPersonResources = false;
            Log("PrimeThirdPersonResources: SpawnPlayer refused (already loading?)");
        }
        return ok;
    }

    inline void __fastcall EntityLoadedDetour(CPlayerSpawnManager* this_)
    {
        if (g_primingThirdPersonResources)
        {
            g_primingThirdPersonResources = false;

            if (auto* loader = this_->m_EntityResourceLoader)
            {
                meow_hook::func_call<void>(GetAddress(RESOURCE_SET_MERGE), &this_->m_ResourceSet, &loader->InstanceSet);
                g_thirdPersonResourcesPrimed = true;
                Log("EntityLoadedDetour: third-person resources primed");
            }
            return;
        }

        g_originalEntityLoaded(this_);
    }

    inline bool InitThirdPersonResourcePriming()
    {
        if (g_originalEntityLoaded) return true;
        return MH_CreateHookGZ(SPAWN_PLAYER_ENTITY_LOADED, &EntityLoadedDetour, &g_originalEntityLoaded);
    }
} // namespace gz
#pragma pack(pop)
