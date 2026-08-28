#pragma once

#include <vector>
#include <utility>
#include "camera_registry.h"

namespace gz
{
    struct SCameraEntry;
}

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

    inline bool g_active = false;
    inline std::vector<std::pair<SCameraId, int32_t>> g_savedPrios;

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

    inline bool Set(bool enable)
    {
        auto* director = CCameraDirector::instance();
        auto* registry = CCameraRegistry::instance();
        if (!director || !registry) return false;

        const auto entries = registry->Enumerate();

        SCameraEntry tp{};
        bool haveTp = false;
        for (const auto& e : entries)
        {
            if (e.m_NameHash == kCameraNameHash) { tp = e; haveTp = true; break; }
        }

        if (enable)
        {
            if (!haveTp)
            {
                Log("ThirdPersonCamera: camera 0x%08X is not registered", kCameraNameHash);
                return false;
            }
            if (g_active) return true;

            ApplyPriorities(entries);
            director->PushCamera(&tp.m_Id);
            g_active = true;
        }
        else
        {
            if (haveTp) director->PopCamera(&tp.m_Id);
            RestorePriorities(entries);
            g_active = false;
        }
        return true;
    }

    [[nodiscard]] inline bool IsActive() { return g_active; }

    /// call when player character changes
    inline void Reset()
    {
        g_savedPrios.clear();
        g_active = false;
    }
}
