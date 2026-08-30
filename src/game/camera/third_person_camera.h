#pragma once

#include <vector>
#include <iterator>
#include <utility>
#include "camera_registry.h"
#include "game/tp_state.h"

namespace gz::ThirdPersonCamera
{
    inline constexpr uint32_t kCameraNameHash = 0x7269447F; // DebugThirdPersonCamera
    inline constexpr int32_t  kThirdPersonPrio = 50;

    /// added to prio of every camera that must beat tp
    inline constexpr int32_t  kOverrideBoost = 100;

    /// only cameras tp is allowed to win against. anything NOT in this list gets boosted (unrecognized cameras override tp rather than breaking)
    inline constexpr const char* kThirdPersonBeats[] = {
        "FirstPersonStandWP",
        "FirstPersonCrouchWP",
        "FirstPersonJogWP",
        "FirstPersonSprintWP",
        "FirstPersonProneWP",

        // TODO: in the future it would be nice to let these overwrite the tp cam
        "IronSightStand",
        "IronSightCrouch",
        "IronSightProne",
        "AimScopeStand",
        "AimScopeCrouch",
        "AimScopeProne",
        "BinocularAimScopeStand",
        "BinocularAimScopeCrouch",
        "BinocularAimScopeProne",
    };

    inline std::vector<std::pair<SCameraId, int32_t>> g_savedPrios;
    inline std::vector<SCameraId> g_beatenIds; // ids tp outranks, cached at enable
    inline SCameraId g_tpId{};

    inline bool ShouldOverrideTp(uint32_t nameHash)
    {
        if (nameHash == kCameraNameHash) return false; // the tp camera itself
        for (const char* name : kThirdPersonBeats)
            if (Utils::HashBuffer(name, 0) == nameHash)
                return false;
        return true;
    }

    /// snapshots every prio, then boosts the override set
    inline void ApplyPriorities(const std::vector<SCameraEntry>& entries)
    {
        g_savedPrios.clear();
        g_savedPrios.reserve(entries.size());

        for (const auto& e : entries)
        {
            if (!e.m_Pipeline) continue;
            if (e.m_NameHash != kCameraNameHash && !ShouldOverrideTp(e.m_NameHash))
                continue; // plain FP camera, leave it alone

            g_savedPrios.emplace_back(e.m_Id, e.m_Pipeline->m_Prio);

            e.m_Pipeline->m_Prio = (e.m_NameHash == kCameraNameHash)
                ? kThirdPersonPrio
                : e.m_Pipeline->m_Prio + kOverrideBoost;
        }
    }

    /// restores every prio by id
    inline void RestorePriorities(const std::vector<SCameraEntry>& entries)
    {
        for (const auto& e : entries)
        {
            if (!e.m_Pipeline) continue;
            for (const auto& [id, prio] : g_savedPrios)
            {
                if (id == e.m_Id)
                {
                    e.m_Pipeline->m_Prio = prio;
                    break;
                }
            }
        }
        g_savedPrios.clear();
    }

    /// caches the ids of every camera tp outranks, so NeedsRepush() is plain comparisons
    inline void CacheBeatenIds(const std::vector<SCameraEntry>& entries)
    {
        g_beatenIds.clear();
        g_beatenIds.reserve(std::size(kThirdPersonBeats));

        for (const auto& e : entries)
        {
            if (e.m_NameHash == kCameraNameHash) continue; // the tp camera itself
            if (ShouldOverrideTp(e.m_NameHash)) continue; // boosted, outranks us
            g_beatenIds.push_back(e.m_Id);
        }
    }

    inline bool NeedsRepush()
    {
        if (!TpState::g_cameraActive) return false;

        auto* director = CCameraDirector::instance();
        if (!director) return false;

        const SCameraId& sel = director->m_SelectedCamera;

        // stack wiped -> m_SelectedCamera is zeroed
        if (sel.m_Hash[0] == 0 && sel.m_Hash[1] == 0 && sel.m_Hash[2] == 0)
            return true;

        if (sel == g_tpId) return false;

        // an fp camera we should be beating is selected -> we fell off the stack
        for (const auto& id : g_beatenIds)
            if (id == sel) return true;

        // anything else is a boosted camera (vehicle, rc, ragdoll), leave it alone
        return false;
    }

    inline bool Reapply()
    {
        auto* registry = CCameraRegistry::instance();
        auto* director = CCameraDirector::instance();
        if (!registry || !director) return false;

        const auto entries = registry->Enumerate();
        RestorePriorities(entries);

        SCameraEntry tp{};
        bool haveTp = false;
        for (const auto& e : entries)
            if (e.m_NameHash == kCameraNameHash)
            {
                tp = e;
                haveTp = true;
                break;
            }

        if (!haveTp)
        {
            Log("ThirdPersonCamera: camera 0x%08X is not registered", kCameraNameHash);
            return false;
        }

        ApplyPriorities(entries);
        CacheBeatenIds(entries);
        g_tpId = tp.m_Id;
        director->PushCamera(&tp.m_Id);
        return true;
    }

    inline bool Set(bool enable)
    {
        if (enable)
        {
            if (TpState::g_cameraActive) return true;
            if (!Reapply()) return false;
            TpState::g_cameraActive = true;
            return true;
        }

        auto* registry = CCameraRegistry::instance();
        auto* director = CCameraDirector::instance();
        if (registry && director)
        {
            const auto entries = registry->Enumerate();
            for (const auto& e : entries)
                if (e.m_NameHash == kCameraNameHash)
                {
                    SCameraId id = e.m_Id;
                    director->PopCamera(&id);
                    break;
                }
            RestorePriorities(entries);
        }

        g_beatenIds.clear();
        g_tpId = SCameraId{};
        TpState::g_cameraActive = false;
        return true;
    }

    /// call when player character changes
    inline void Reset()
    {
        g_savedPrios.clear();
        g_beatenIds.clear();
        g_tpId = SCameraId{};
        TpState::g_cameraActive = false;
    }

    [[nodiscard]] inline bool IsActive() { return TpState::g_cameraActive; }
}
