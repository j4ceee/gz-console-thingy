#pragma once

#include "log.h"
#include "meow_hook/util.h"
#include "game/character.h"
#include "game/util/hash_utils.h"
#include "game/custom/free_cam_state.h"
#include "game/custom/tp_state.h"
#include "game/custom/third_person_camera.h"

#pragma pack(push, 1)
namespace gz
{
    /// Third-person animation layer set<br>
    /// - swaps local player animation layers between the fp set (local_player_character.ee) & tp set (tp_local_player_character.ee) at runtime
    /// - limit: m_InactiveLayers[8] and m_InactiveBodyParts[8] are fixed 8-byte arrays, so 8 is the hard max
    namespace TpLayers
    {
        inline constexpr int kSwapCount = 7; // stock fp layer count (slots 0 - 6)
        inline constexpr int kTotalCount = 8; // tp layer count
        inline constexpr int kTpFileCount = 8; // entries in kFiles
        inline constexpr int kSpareSlot = 7; // holds a parked tp layer in fp mode

        struct LayerFiles
        {
            const char* asb;
            const char* afsmb;
        };

#define GZ_TP_SM(name) \
        { "animations/statemachines/humans/player_thirdperson_" name ".asb", \
          "animations/statemachines/humans/player_thirdperson_" name ".afsmb" }

        // index here is a file index, referenced by kTpPlan - not a slot
        inline constexpr LayerFiles kFiles[kTpFileCount] = {
            GZ_TP_SM("fullbody"),                   // 0
            GZ_TP_SM("upperbody"),                  // 1
            GZ_TP_SM("weapon_handling_additive"),   // 2
            GZ_TP_SM("weapon_recoil_additive"),     // 3
            GZ_TP_SM("reaction_additive"),          // 4
            GZ_TP_SM("face"),                       // 5
            GZ_TP_SM("control_extras"),             // 6 - unused, see kTpPlan
            GZ_TP_SM("control"),                    // 7 - unused, see kTpPlan
        };

#undef GZ_TP_SM

        enum class Src : uint8_t { Tp, Fp };

        struct SlotPlan
        {
            Src src;
            int index; // Src::Tp -> index into kFiles; Src::Fp -> stock FP slot
        };

        /// tp mode layout. both fp entries are mandatory and were verified by testing:
        /// - replacing slot 6 with tp control_extras breaks shooting
        /// - replacing slot 7 with tp control breaks movement (sprint, jump, crouch, prone)
        inline constexpr SlotPlan kTpPlan[kTotalCount] = {
            {Src::Tp, 0}, // 0 MAINBODY           TP fullbody
            {Src::Tp, 1}, // 1 UPPERBODY          TP upperbody
            {Src::Tp, 2}, // 2 GLOBAL_PARTIAL     TP weapon_handling_additive
            {Src::Tp, 3}, // 3 SYNCED_ADDITIVE    TP weapon_recoil_additive
            {Src::Tp, 4}, // 4 GLOBAL_ADDITIVE    TP reaction_additive
            {Src::Tp, 5}, // 5 VOCALS             TP face
            {Src::Fp, 1}, // 6                    FP UPPERBODY (weapons, items, ADS)
            {Src::Fp, 0}, // 7                    FP MAINBODY  (movement, sprint, jump)
        };

        constexpr int CountSrc(Src s)
        {
            int n = 0;
            for (const auto& p : kTpPlan) if (p.src == s) ++n;
            return n;
        }

        // name hash that already lives on each target slot in the stock FP set
        // slot 7 does not exist until GrowToEight() runs
        inline constexpr uint32_t kUnnamedHash = 0xDEADBEEF;
        inline constexpr uint32_t kSlotHashes[kTotalCount] = {
            0x9011E763, // 0 MAINBODY
            0xA6468F52, // 1 UPPERBODY
            0x457905E1, // 2 GLOBAL_PARTIAL
            0xB2BE2993, // 3 SYNCED_ADDITIVE
            0x85E439F0, // 4 GLOBAL_ADDITIVE
            0x5A9EF178, // 5 VOCALS
            kUnnamedHash, // 6
            kUnnamedHash, // 7
        };

        inline SAnimationLayerInstance g_fpParked[kSwapCount]{}; // indexed by stock FP slot
        inline SAnimationLayerInstance g_tpParked[kTpFileCount]{}; // indexed by kFiles index

        // --- layer plumbing, all of it operates on the local player character ---

        /// Builds a fresh SAnimationLayerInstance for <code>kFiles[fileIndex]</code> into <code>out</code>
        /// - tagged with name hash & index of slot it will occupy
        /// - returns false if the resources aren't in the cache
        inline bool BuildInstance(int fileIndex, int targetSlot, SAnimationLayerInstance* out)
        {
            const auto& files = kFiles[fileIndex];

            alignas(8) SAnimationLayerInfo info{};
            info.m_AfsmFileName.SetTemporary(files.afsmb);
            info.m_AsFileName.SetTemporary(files.asb);
            info.m_AfsmFileHash = Utils::HashString(files.afsmb);
            info.m_AsFileHash = Utils::HashString(files.asb);
            info.m_LayerHash = kSlotHashes[targetSlot];
            info.m_LayerIndex = targetSlot;

            alignas(8) SAnimationLayerInstance tmp{};
            meow_hook::func_call<void>(GetAddress(ANIM_LAYER_CONST), &tmp, &info);

            info.m_AfsmFileName.FreeTemporary();
            info.m_AsFileName.FreeTemporary();

            if (!tmp.IsResolved())
            {
                Log("TpLayers: '%s' failed to resolve (sm=%p animSet=%p)",
                    files.afsmb, tmp.m_StateMachine.px, tmp.m_AnimSetHandle);
                meow_hook::func_call<void>(GetAddress(ANIM_LAYER_DEST), &tmp);
                return false;
            }

            *out = tmp;
            return true;
        }

        /// - moves the instance out of <code>m_DefaultLayers[index]</code> into <code>dst</code>
        /// - slot is zeroed, not destroyed; ownership transfers to <code>dst</code>
        inline void MoveOutOfSlot(CCharacter* c, int index, SAnimationLayerInstance* dst)
        {
            auto* slot = c->GetAnimatedModel()->GetDefaultLayerSlot(index);
            if (!slot) return;
            *dst = *slot;
            memset(slot, 0, sizeof(*slot));
        }

        /// - moves <code>src</code> into <code>m_DefaultLayers[index]</code>, destroying whatever was there
        /// - <code>src</code> is zeroed so the same instance can never be installed twice
        inline void MoveIntoSlot(CCharacter* c, int index, SAnimationLayerInstance* src)
        {
            auto* slot = c->GetAnimatedModel()->GetDefaultLayerSlot(index);
            if (!slot) return;
            if (slot->m_StateMachine.px) // only destroy a live instance
                meow_hook::func_call<void>(GetAddress(ANIM_LAYER_DEST), slot);
            *slot = *src;
            memset(src, 0, sizeof(*src));
            slot->m_LayerIndex = index;
        }

        /// grows m_DefaultLayers from 7 to 8
        /// - appended instance is a placeholder, it gets replaced before InitializeRuleSystems runs
        /// - INSTALL ONLY, call once & never call again!
        inline bool GrowToEight(CCharacter* c)
        {
            auto* model = c->GetAnimatedModel();
            const int count = model->GetDefaultLayerCount();
            if (count == kTotalCount) return true;
            if (count != kTotalCount - 1)
            {
                Log("TpLayers: unexpected layer count %d", count);
                return false;
            }

            const auto& files = kFiles[0]; // fullbody, known resolvable

            alignas(8) SAnimationLayerInfo info{};
            info.m_AfsmFileName.SetTemporary(files.afsmb);
            info.m_AsFileName.SetTemporary(files.asb);
            info.m_AfsmFileHash = Utils::HashString(files.afsmb);
            info.m_AsFileHash = Utils::HashString(files.asb);
            info.m_LayerHash = kUnnamedHash;
            info.m_LayerIndex = kSpareSlot;

            meow_hook::func_call<void>(GetAddress(ANIM_MODEL_ADD_LAYER), model, &info);
            info.m_AfsmFileName.FreeTemporary();
            info.m_AsFileName.FreeTemporary();

            auto* slot = model->GetDefaultLayerSlot(kSpareSlot);
            if (!slot || !slot->m_StateMachine.px)
            {
                Log("TpLayers: could not grow to 8 layers");
                if (slot) // AddAnimationLayer appends unconditionally - roll it back
                {
                    meow_hook::func_call<void>(GetAddress(ANIM_LAYER_DEST), slot);
                    model->m_DefaultLayers.PopBackRaw(sizeof(SAnimationLayerInstance));
                }
                return false;
            }
            return true;
        }

        /// Rebuilds rule systems, body parts and per-slot state-task memory from the current contents of m_DefaultLayers.
        /// Required after any layer change.
        inline void RebuildRuleSystems(CCharacter* c)
        {
            meow_hook::func_call<void>(GetAddress(ANIM_MODEL_INIT_RULE_SYSTEMS),
                                       c->GetAnimatedModel(),
                                       "animations/skeletons/characters/humans/sk100.bsk",
                                       GetAddress(CHARACTER_ON_STATE_TRANSITION),
                                       c->GetGameObject());
        }

        /// skips driver & pose contribution
        inline void SetLayerActive(CCharacter* c, int index, bool active)
        {
            auto* ctrl = c->GetAnimatedModel()->GetAnimationControl();
            if (!ctrl || index < 0 || index >= ctrl->GetBodyPartCount()) return;

            ctrl->m_InactiveBodyParts[index] = !active;
            if (!active)
                if (void* blender = ctrl->GetBlender(index))
                    meow_hook::func_call<void>(GetAddress(BLEND_TREE_CLEAR), blender);
        }

        /// Fully parks a layer we are only storing, not using.
        ///
        /// 2 flags:
        /// - CAnimationControl::m_InactiveBodyParts[i] - skips driver & pose contribution (SetLayerActive)
        /// - CAnimatedModel::m_InactiveLayers[i] - gates UpdateRuleSystem / the STATE MACHINE itself (SetLayerParked)
        ///
        /// NEVER park a layer whose tasks are still needed (the FP logic layers)
        inline void SetLayerParked(CCharacter* c, int index, bool parked)
        {
            auto* model = c->GetAnimatedModel();
            if (index < 0 || index >= 8) return;
            model->m_InactiveLayers[index] = parked;
            SetLayerActive(c, index, !parked);
        }

        /// clears every inactivation flag on BOTH arrays. Call AFTER RebuildRuleSystems() and before applying the per-mode flags
        /// - CAnimationControl::m_InactiveBodyParts[i] - skips the driver & pose contribution
        /// - CAnimatedModel::m_InactiveLayers[i]       - gates UpdateRuleSystem / the state machine
        inline void ResetLayerFlags(CCharacter* c)
        {
            auto* model = c->GetAnimatedModel();
            auto* ctrl = model->GetAnimationControl();
            if (!ctrl) return;

            for (int i = 0; i < 8; ++i)
            {
                model->m_InactiveLayers[i] = false;
                ctrl->m_InactiveBodyParts[i] = false;
            }
        }

        /// tp mode: fp logic layers at 6/7 are hidden from the pose only
        /// - their rule systems must keep running -> handles sprint, crouch, prone & weapon tasks
        inline void ApplyTpLayerFlags(CCharacter* c)
        {
            ResetLayerFlags(c);
            for (int i = 0; i < kTotalCount; ++i)
                if (kTpPlan[i].src == Src::Fp)
                    SetLayerActive(c, i, false); // logic runs, pose hidden
        }

        /// fp mode: everything is stock except the spare TP layer in slot 7 which is fully parked (pose & state machine)
        inline void ApplyFpLayerFlags(CCharacter* c)
        {
            ResetLayerFlags(c);
            SetLayerParked(c, kSpareSlot, true);
        }

        /// Dumps the current layer table
        inline void LogLayerState(CCharacter* c, const char* tag)
        {
            auto* model = c->GetAnimatedModel();
            auto* ctrl = model->GetAnimationControl();
            const int count = model->GetDefaultLayerCount();

            Log("TpLayers[%s]: %d layers, %d rule systems, %d body parts, %d layer infos",
                tag, count, model->GetRuleSystemCount(), ctrl ? ctrl->GetBodyPartCount() : -1, model->GetLayerInfoCount());

            for (int i = 0; i < count; ++i)
            {
                const auto* slot = model->GetDefaultLayerSlot(i);
                if (!slot) continue;
                const auto* hashes = model->GetCurrentLayerInfo(i);
                Log("  [%d] sm=%p animSet=%p afsm=%08X as=%08X name=%08X idx=%d inactiveLayer=%d inactiveBody=%d",
                    i,
                    slot->m_StateMachine.px,
                    slot->m_AnimSetHandle,
                    hashes ? hashes->m_AfsmFileHash : 0,
                    hashes ? hashes->m_AsFileHash : 0,
                    slot->m_LayerHash,
                    slot->m_LayerIndex,
                    (i < 8) ? (model->m_InactiveLayers[i] ? 1 : 0) : -1,
                    (ctrl && i < 8) ? (ctrl->m_InactiveBodyParts[i] ? 1 : 0) : -1);
            }
        }

        /// - parked instances belong to one specific character and die with it
        /// - if the player character changed, drop the install state, do not run destructor on old instances (already gone)
        /// - neither character nor world change from the main menu trigger this, but let's keep it to be sure
        inline void ResetIfCharacterChanged(CCharacter* c)
        {
            if (TpState::g_layerOwner == c) return;
            if (TpState::g_layersInstalled)
                Log("TpLayers: character changed (%p -> %p), resetting install state", TpState::g_layerOwner, c);

            memset(g_fpParked, 0, sizeof(g_fpParked));
            memset(g_tpParked, 0, sizeof(g_tpParked));
            ThirdPersonCamera::Reset();
            TpState::g_layersInstalled = false;
            TpState::g_animationsActive = false;
            TpState::g_layerOwner = c;
        }

        /// installs the tp layout. <code>tpSource</code> is file-indexed, so it takes either <code>staged</code> array or parked g_tpParked buffer
        inline void ApplyPlan(CCharacter* c, SAnimationLayerInstance* tpSource)
        {
            for (int i = 0; i < kTotalCount; ++i)
                MoveIntoSlot(c, i, kTpPlan[i].src == Src::Tp ? &tpSource[kTpPlan[i].index] : &g_fpParked[kTpPlan[i].index]);
        }

        /// First time install
        inline bool Install(CCharacter* c)
        {
            ResetIfCharacterChanged(c);
            if (TpState::g_layersInstalled) return true;

            auto* model = c->GetAnimatedModel();
            if (!model->GetAnimationControl())
            {
                Log("TpLayers: no animation controller");
                return false;
            }
            if (model->GetDefaultLayerCount() != kTotalCount - 1)
            {
                Log("TpLayers: expected %d stock layers, found %d",
                    kTotalCount - 1, model->GetDefaultLayerCount());
                return false;
            }

            // build every tp machine the plan asks for up front
            alignas(8) SAnimationLayerInstance staged[kTpFileCount]{};
            bool built[kTpFileCount]{};
            for (int i = 0; i < kTotalCount; ++i)
            {
                const auto& p = kTpPlan[i];
                if (p.src != Src::Tp || built[p.index]) continue;
                if (!BuildInstance(p.index, i, &staged[p.index]))
                {
                    for (int j = 0; j < kTpFileCount; ++j)
                        if (built[j]) meow_hook::func_call<void>(GetAddress(ANIM_LAYER_DEST), &staged[j]);
                    Log("TpLayers: install aborted, nothing changed");
                    return false;
                }
                built[p.index] = true;
            }

            if (!GrowToEight(c))
            {
                for (int j = 0; j < kTpFileCount; ++j)
                    if (built[j]) meow_hook::func_call<void>(GetAddress(ANIM_LAYER_DEST), &staged[j]);
                return false;
            }

            // park the stock fp layers, then lay out the plan (slot 7 currently holds GrowToEight placeholder, which ApplyPlan destroys)
            for (int i = 0; i < kSwapCount; ++i)
                MoveOutOfSlot(c, i, &g_fpParked[i]);

            ApplyPlan(c, staged);

            RebuildRuleSystems(c);
            ApplyTpLayerFlags(c);

            TpState::g_layersInstalled = true;
            TpState::g_animationsActive = true;
            Log("TpLayers: installed (%d TP render, %d FP logic layers)", CountSrc(Src::Tp), CountSrc(Src::Fp));
            // LogLayerState(c, "installed");
            return true;
        }

        /// Toggles between the fp and tp layer sets
        /// - layer count stays at 8 in both directions
        /// - in fp mode one tp layer is parked (hidden) in spare slot rather than shrinking m_DefaultLayers
        inline bool SetAnimations(CCharacter* c, bool enable)
        {
            ResetIfCharacterChanged(c);
            if (!TpState::g_layersInstalled) return enable ? Install(c) : false;
            if (TpState::g_animationsActive == enable) return true;

            auto* model = c->GetAnimatedModel();
            if (model->GetDefaultLayerCount() != kTotalCount)
            {
                Log("TpLayers: layer count is %d, refusing to swap",
                    model->GetDefaultLayerCount());
                return false;
            }

            // LogLayerState(c, enable ? "before FP->TP" : "before TP->FP");

            // whichever TP file sits at slot 0 is the one parked in the spare slot
            static_assert(kTpPlan[0].src == Src::Tp, "spare slot parks a TP layer");
            constexpr int kSpareFile = kTpPlan[0].index;

            if (!enable) // TP -> FP
            {
                for (int i = 0; i < kTotalCount; ++i)
                {
                    const auto& p = kTpPlan[i];
                    MoveOutOfSlot(c, i, p.src == Src::Tp ? &g_tpParked[p.index] : &g_fpParked[p.index]);
                }

                for (int i = 0; i < kSwapCount; ++i)
                    MoveIntoSlot(c, i, &g_fpParked[i]);

                MoveIntoSlot(c, kSpareSlot, &g_tpParked[kSpareFile]); // keeps count at 8

                RebuildRuleSystems(c);
                ApplyFpLayerFlags(c);
            }
            else // FP -> TP
            {
                MoveOutOfSlot(c, kSpareSlot, &g_tpParked[kSpareFile]);

                for (int i = 0; i < kSwapCount; ++i)
                    MoveOutOfSlot(c, i, &g_fpParked[i]);

                ApplyPlan(c, g_tpParked);

                RebuildRuleSystems(c);
                ApplyTpLayerFlags(c);
            }

            TpState::g_animationsActive = enable;
            // LogLayerState(c, enable ? "after FP->TP" : "after TP->FP");
            return true;
        }
    }

    /// Third person facade, everything outside this file goes through here
    namespace ThirdPerson
    {
        /// Sets the visibility of the third person character body
        inline void SetBodyVisible(CCharacter* c, bool visible)
        {
            c->SetThirdPersonBodyVisible(visible);
        }

        /// why third person can't be toggled right now, nullptr if it can
        [[nodiscard]] inline const char* GetBlockReason(CCharacter* c)
        {
            if (FreeCamState::IsInUse()) return "Not available in free cam.";
            if (!TpState::g_resourcesPrimed) return "Third person is still loading.";
            return c->GetControlBlockReason(); // crashes the game otherwise
        }

        inline void Set(CCharacter* c, bool enable)
        {
            if (const char* why = GetBlockReason(c))
            {
                Log("Third person: %s", why);
                return;
            }

            TpLayers::SetAnimations(c, enable);
            SetBodyVisible(c, enable);
            ThirdPersonCamera::Set(enable);
        }
    }
} // namespace gz
#pragma pack(pop)
