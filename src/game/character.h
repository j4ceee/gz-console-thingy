#pragma once

#include "animation_control.h"
#include "remote_controller.h"
#include "data_types.h"
#include "game_object.h"
#include "meow_hook/util.h"
#include "util/hash_utils.h"
#include "../log.h"
#include "custom/tp_state.h"
#include "custom/third_person_camera.h"

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
            { Src::Tp, 0 }, // 0 MAINBODY           TP fullbody
            { Src::Tp, 1 }, // 1 UPPERBODY          TP upperbody
            { Src::Tp, 2 }, // 2 GLOBAL_PARTIAL     TP weapon_handling_additive
            { Src::Tp, 3 }, // 3 SYNCED_ADDITIVE    TP weapon_recoil_additive
            { Src::Tp, 4 }, // 4 GLOBAL_ADDITIVE    TP reaction_additive
            { Src::Tp, 5 }, // 5 VOCALS             TP face
            { Src::Fp, 1 }, // 6                    FP UPPERBODY (weapons, items, ADS)
            { Src::Fp, 0 }, // 7                    FP MAINBODY  (movement, sprint, jump)
        };

        constexpr int CountSrc(Src s)
        {
            int n = 0;
            for (const auto& p : kTpPlan) if (p.src == s) ++n;
            return n;
        }

        // name hash that already lives on each target slot in the stock FP set
        // slot 7 does not exist until GrowToEightLayers() runs
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
    }

    class CDeepWaterHandling;
    class CAnimalCharacterComponent;
    class CDamageable;
    class CAvatar;

    struct CObjectBlackboard
    {
        void*       m_KeyInfos;         // +0x00
        uint16_t    m_KeyInfoCapacity;  // +0x08
        uint8_t     _pad1[6];           // +0x0A
        void*       m_Data;             // +0x10
        uint16_t    m_DataCapacity;     // +0x18
        uint16_t    m_CurrentOffset;    // +0x1A
        uint8_t     _pad2[4];           // +0x1C
        void*       m_NetCallback;      // +0x20
        void*       m_NetUserData;      // +0x28
        void*       m_BbLock;           // +0x30
    };
    static_assert(sizeof(CObjectBlackboard) == 0x38);

    class CPfxCharacterInstance
    {
    public:
        char        _pad[0x38];                     // 0x00 → 0x38
        bool        m_InputAirJump;                 // 0x38
        bool        m_InputJump;                    // 0x39
        bool        m_InputGameControlledVelocity;  // 0x3A
        char        _pad0[0x11];                    // 0x3B → 0x4C
        CVector3f   m_WantedVelocityWS;             // 0x4C → 0x58
        char        _pad1[0x68];                    // 0x58 → 0xC0
        float       m_GroundDistance;               // 0xC0 → 0xC4
        char        _pad2[0x38];                    // 0xC4 → 0xFC
        float       m_Gravity;                      // 0xFC → 0x100

        [[nodiscard]] float GetGravity() const { return m_Gravity; }
        [[nodiscard]] float GetGroundDistance() const { return m_GroundDistance; }

        [[nodiscard]] float GetGravityInGs() const
        {
            return m_Gravity / -9.810f;
        }

        void SetGravityInGs(float gs)
        {
            m_Gravity = gs * -9.810f;
        }

        void ResetGravityForPlayer()
        {
            m_Gravity = -21; // default gravity value for the player character
        }
    };
    static_assert(offsetof(CPfxCharacterInstance, m_InputJump) == 0x39);
    static_assert(offsetof(CPfxCharacterInstance, m_WantedVelocityWS) == 0x4C);
    static_assert(sizeof(CPfxCharacterInstance) == 0x100);

    class CCharacter
    {
    public:
        char                        _pad_damageable[0x2C4]; // 0x0000 → 0x02C4 (CDamageable)
        char                        _pad1[0x5E4];           // 0x02C4 → 0x08A8
        int                         m_originalFaction;      // 0x08A8 → 0x08AC
        char                        _pad2[0x2C];            // 0x08AC → 0x08D8
        void**                      m_interactionData;      // 0x08D8 → 0x08E0 - pointer to interaction data
        char                        _pad3[0x990];           // 0x08E0 → 0x1270
        CAnimatedModel              m_animatedModel;        // 0x1270 → 0x13C0
        char                        _pad4[0x1FE0];          // 0x13C0 → 0x33A0
        CPfxCharacterInstance*      m_pfxInstance;          // 0x33A0 → 0x33A8
        char                        _pad5[0x8];             // 0x33A8 → 0x33B0
        CObjectBlackboard           m_Blackboard;           // 0x33B0 → 0x33E8
        char                        _pad6[0x8];             // 0x33E8 → 0x33F0
        CRemoteController*          m_remoteController;     // 0x33F0 → 0x33F8
        char                        _pad7[0x10];            // 0x33F8 → 0x3408
        bool                        m_unlimitedAmmo;        // 0x3408 → 0x3409 (for player it still consumes inventory ammo)
        uint8_t                     m_controlFlag;          // 0x3409 → 0x340A
        char                        _pad8a[0x3C2];          // 0x340A → 0x37CC
        uint8_t                     m_motionState;          // 0x37CC → 0x37CD  (5 = POSITIONING, no physics)
        uint8_t                     m_defaultMotionState;   // 0x37CD → 0x37CE
        char                        _pad8b[0x126];          // 0x37CE → 0x38F4
        bool                        m_detectable;           // 0x38F4 → 0x38F5
        char                        _pad9[0xEF];            // 0x38F5 → 0x39E4
        CVector2f                   m_currentGravity;       // 0x39E4 → 0x39EC (array of 2 floats)
        char                        _pad10[0xC];            // 0x39EC → 0x39F8
        int                         m_faction;              // 0x39F8 → 0x39FC
        char                        _pad11[0xAC];           // 0x39FC → 0x3AA8
        CMatrix4f                   m_worldMatrix;          // 0x3AA8 → 0x3AE8  (0x40 bytes)
        char                        _pad12[0x140];          // 0x3AE8 → 0x3C28
        CAvatar*                    m_avatar;               // 0x3C28 → 0x3C30
        char                        _pad13[0x758];          // 0x3C30 → 0x4388
        CAnimalCharacterComponent*  m_animalComponent;      // 0x4388 → 0x4390
        char                        _pad14[0x10];           // 0x4390 → 0x43A0
        CDeepWaterHandling*         m_deepWaterHandling;    // 0x43A0 → 0x43A8

        [[nodiscard]] CDamageable* GetDamageable()
        {
            return reinterpret_cast<CDamageable*>(this);
        }

        [[nodiscard]] CPfxCharacterInstance* GetPfxInstance()
        {
            return m_pfxInstance;
        }

        /// CCharacter's CGameObject subobject starts at +0x08.
        /// Byte arithmetic on purpose: CGameObject is currently an empty class, so
        /// `reinterpret_cast<CGameObject*>(this) + 8` would scale by sizeof(CGameObject)
        /// and silently break the moment a member is added to it.
        [[nodiscard]] CGameObject* GetGameObject()
        {
            return reinterpret_cast<CGameObject*>(reinterpret_cast<uint8_t*>(this) + 8);
        }

        /// <summary>
        /// Gets the animal component of the character, if it has one.
        /// </summary>
        /// <returns>A pointer to the animal component, or nullptr if the character does not have one.</returns>
        [[nodiscard]] CAnimalCharacterComponent* GetAnimalComponent()
        {
            return m_animalComponent;
        }

        [[nodiscard]] CAnimatedModel* GetAnimatedModel()
        {
            return &m_animatedModel;
        }

        [[nodiscard]] CRemoteController* GetRemoteController() const
        {
            return m_remoteController;
        }

        [[nodiscard]] int GetOriginalFaction() const { return m_originalFaction; }
        [[nodiscard]] int GetFaction() const { return m_faction; }

        void SetFaction(int faction) { m_faction = faction; }
        void ResetFaction() { m_faction = m_originalFaction; }

        [[nodiscard]] bool IsDetectable() const { return m_detectable; }
        void SetDetectable(bool detectable) { m_detectable = detectable; }

        [[nodiscard]] CVector3f GetPosition() const
        {
            return { m_worldMatrix.m[3].x, m_worldMatrix.m[3].y, m_worldMatrix.m[3].z };
        }

        [[nodiscard]] CVector3f GetForward() const
        {
            return { m_worldMatrix.m[2].x, m_worldMatrix.m[2].y, m_worldMatrix.m[2].z };
        }

        [[nodiscard]] CVector3f GetRight() const
        {
            return { m_worldMatrix.m[0].x, m_worldMatrix.m[0].y, m_worldMatrix.m[0].z };
        }

        [[nodiscard]] CVector3f GetUp() const
        {
            return { m_worldMatrix.m[1].x, m_worldMatrix.m[1].y, m_worldMatrix.m[1].z };
        }

        void SetPosition(float x, float y, float z)
        {
            m_worldMatrix.m[3] = { x, y, z, 1.0f };
        }

        void SetPosition(const CVector3f& pos)
        {
            m_worldMatrix.m[3] = { pos.x, pos.y, pos.z, 1.0f };
        }

        void SetMotionState(uint8_t state) { m_motionState = state; }

        [[nodiscard]] bool IsSoviet() const { return m_originalFaction == 5; }
        [[nodiscard]] bool IsFNIX() const { return m_originalFaction == 2; }
        [[nodiscard]] bool IsResistance() const { return m_originalFaction == 0; }

        /// <summary>
        /// Get the distance to another character in meters.
        /// </summary>
        float GetDistanceTo(const CCharacter* other) const
        {
            if (!other) return -1.0f;

            CVector3f myPos = GetPosition();
            CVector3f otherPos = other->GetPosition();

            float dx = myPos.x - otherPos.x;
            float dy = myPos.y - otherPos.y;
            float dz = myPos.z - otherPos.z;

            return sqrtf(dx*dx + dy*dy + dz*dz);
        }

        [[nodiscard]] float GetInteractionRadius() const
        {
            // double-pointer dereference to get radius
            if (!m_interactionData || !*m_interactionData) {
                return 0.0f;
            }
            const auto* radiusPtr = static_cast<float*>(*m_interactionData);
            return *radiusPtr;
        }

        /// <summary>
        /// Check if the character is within interaction range of another character.\n
        /// e.g. check if playerChar (this) is within interaction range of a machineChar (other)
        /// </summary>
        bool IsWithinInteractionRangeOf(const CCharacter* other) const
        {
            if (!other) return false;

            float radius = other->GetInteractionRadius();
            if (radius <= 0.0f) return false;

            float distance = GetDistanceTo(other);

            return distance <= (radius * 0.5f); // within half the interaction radius
        }

        void Revive()
        {
            meow_hook::func_call<void>(
                GetAddress(CHARACTER_REVIVE),
                this,
                true,       // call CDamageable::RestoreHealth()
                false,      // unknown
                1.0f        // health percentage
            );
        }

        void SetGhostMode(bool enable)
        {
            meow_hook::func_call<void>(
                GetAddress(CHARACTER_GHOST_MODE),
                this,
                enable
            );
        }

        [[nodiscard]] bool IsControllingEntity() const
        {
            return (m_controlFlag & 0x40) != 0;
        }

        /// <summary>
        /// Sets the visibility of the third person character body
        /// </summary>
        void SetThirdPersonBodyVisible(bool visible)
        {
            meow_hook::func_call<void>(
                GetAddress(FUNC_SET_BLACKBOARD_INT_GET_WRAPPER),
                &m_Blackboard,
                0x41df1e71,
                visible ? 1 : 0,
                0,
                0
            );
        }

        // --- third person stuff -----------------------------------------
    private:
        /// Builds a fresh SAnimationLayerInstance for <code>kFiles[fileIndex]</code> into <code>out</code>
        /// - tagged with name hash & index of slot it will occupy
        /// - returns false if the resources aren't in the cache
        static bool BuildTpLayerInstance(int fileIndex, int targetSlot, SAnimationLayerInstance* out)
        {
            const auto& files = TpLayers::kFiles[fileIndex];

            alignas(8) SAnimationLayerInfo info{};
            info.m_AfsmFileName.SetTemporary(files.afsmb);
            info.m_AsFileName.SetTemporary(files.asb);
            info.m_AfsmFileHash = Utils::HashString(files.afsmb);
            info.m_AsFileHash = Utils::HashString(files.asb);
            info.m_LayerHash = TpLayers::kSlotHashes[targetSlot];
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
        void MoveOutOfSlot(int index, SAnimationLayerInstance* dst)
        {
            auto* slot = GetAnimatedModel()->GetDefaultLayerSlot(index);
            if (!slot) return;
            *dst = *slot;
            memset(slot, 0, sizeof(*slot));
        }

        /// - moves <code>src</code> into <code>m_DefaultLayers[index]</code>, destroying whatever was there
        /// - <code>src</code> is zeroed so the same instance can never be installed twice
        void MoveIntoSlot(int index, SAnimationLayerInstance* src)
        {
            auto* slot = GetAnimatedModel()->GetDefaultLayerSlot(index);
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
        bool GrowToEightLayers()
        {
            auto* model = GetAnimatedModel();
            const int count = model->GetDefaultLayerCount();
            if (count == TpLayers::kTotalCount) return true;
            if (count != TpLayers::kTotalCount - 1)
            {
                Log("TpLayers: unexpected layer count %d", count);
                return false;
            }

            const auto& files = TpLayers::kFiles[0]; // fullbody, known resolvable

            alignas(8) SAnimationLayerInfo info{};
            info.m_AfsmFileName.SetTemporary(files.afsmb);
            info.m_AsFileName.SetTemporary(files.asb);
            info.m_AfsmFileHash = Utils::HashString(files.afsmb);
            info.m_AsFileHash = Utils::HashString(files.asb);
            info.m_LayerHash = TpLayers::kUnnamedHash;
            info.m_LayerIndex = TpLayers::kSpareSlot;

            meow_hook::func_call<void>(GetAddress(ANIM_MODEL_ADD_LAYER), model, &info);
            info.m_AfsmFileName.FreeTemporary();
            info.m_AsFileName.FreeTemporary();

            auto* slot = model->GetDefaultLayerSlot(TpLayers::kSpareSlot);
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
        void RebuildRuleSystems()
        {
            meow_hook::func_call<void>(GetAddress(ANIM_MODEL_INIT_RULE_SYSTEMS),
                                       GetAnimatedModel(),
                                       "animations/skeletons/characters/humans/sk100.bsk",
                                       GetAddress(CHARACTER_ON_STATE_TRANSITION),
                                       GetGameObject());
        }

        /// skips driver & pose contribution
        void SetLayerActive(int index, bool active)
        {
            auto* ctrl = GetAnimatedModel()->GetAnimationControl();
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
        void SetLayerParked(int index, bool parked)
        {
            auto* model = GetAnimatedModel();
            if (index < 0 || index >= 8) return;
            model->m_InactiveLayers[index] = parked;
            SetLayerActive(index, !parked);
        }

        /// clears every inactivation flag on BOTH arrays. Call AFTER RebuildRuleSystems() and before applying the per-mode flags
        /// - CAnimationControl::m_InactiveBodyParts[i] - skips the driver & pose contribution
        /// - CAnimatedModel::m_InactiveLayers[i]       - gates UpdateRuleSystem / the state machine
        void ResetLayerFlags()
        {
            auto* model = GetAnimatedModel();
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
        void ApplyTpLayerFlags()
        {
            using namespace TpLayers;
            ResetLayerFlags();
            for (int i = 0; i < kTotalCount; ++i)
                if (kTpPlan[i].src == Src::Fp)
                    SetLayerActive(i, false); // logic runs, pose hidden
        }

        /// fp mode: everything is stock except the spare TP layer in slot 7 which is fully parked (pose & state machine)
        void ApplyFpLayerFlags()
        {
            ResetLayerFlags();
            SetLayerParked(TpLayers::kSpareSlot, true);
        }

        /// Dumps the current layer table
        void LogLayerState(const char* tag)
        {
            auto* model = GetAnimatedModel();
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
        void ResetIfCharacterChanged()
        {
            using namespace TpLayers;
            if (TpState::g_layerOwner == this) return;
            if (TpState::g_layersInstalled)
                Log("TpLayers: character changed (%p -> %p), resetting install state", TpState::g_layerOwner, this);

            memset(g_fpParked, 0, sizeof(g_fpParked));
            memset(g_tpParked, 0, sizeof(g_tpParked));
            ThirdPersonCamera::Reset();
            TpState::g_layersInstalled = false;
            TpState::g_animationsActive = false;
            TpState::g_layerOwner = this;
        }

        /// installs the tp layout. <code>tpSource</code> is file-indexed, so it takes either <code>staged</code> array or parked g_tpParked buffer
        void ApplyPlan(SAnimationLayerInstance* tpSource)
        {
            using namespace TpLayers;
            for (int i = 0; i < kTotalCount; ++i)
                MoveIntoSlot(i, kTpPlan[i].src == Src::Tp ? &tpSource[kTpPlan[i].index] : &g_fpParked[kTpPlan[i].index]);
        }

        /// First time install
        bool InstallThirdPersonLayers()
        {
            using namespace TpLayers;
            ResetIfCharacterChanged();
            if (TpState::g_layersInstalled) return true;

            auto* model = GetAnimatedModel();
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
                if (!BuildTpLayerInstance(p.index, i, &staged[p.index]))
                {
                    for (int j = 0; j < kTpFileCount; ++j)
                        if (built[j]) meow_hook::func_call<void>(GetAddress(ANIM_LAYER_DEST), &staged[j]);
                    Log("TpLayers: install aborted, nothing changed");
                    return false;
                }
                built[p.index] = true;
            }

            if (!GrowToEightLayers())
            {
                for (int j = 0; j < kTpFileCount; ++j)
                    if (built[j]) meow_hook::func_call<void>(GetAddress(ANIM_LAYER_DEST), &staged[j]);
                return false;
            }

            // park the stock fp layers, then lay out the plan (slot 7 currently holds GrowToEightLayers placeholder, which ApplyPlan destroys)
            for (int i = 0; i < kSwapCount; ++i)
                MoveOutOfSlot(i, &g_fpParked[i]);

            ApplyPlan(staged);

            RebuildRuleSystems();
            ApplyTpLayerFlags();

            TpState::g_layersInstalled = true;
            TpState::g_animationsActive = true;
            Log("TpLayers: installed (%d TP render, %d FP logic layers)", CountSrc(Src::Tp), CountSrc(Src::Fp));
            // LogLayerState("installed");
            return true;
        }

        /// Toggles between the fp and tp layer sets
        /// - layer count stays at 8 in both directions
        /// - in fp mode one tp layer is parked (hidden) in spare slot rather than shrinking m_DefaultLayers
        bool SetThirdPersonAnimations(bool enable)
        {
            using namespace TpLayers;
            ResetIfCharacterChanged();
            if (!TpState::g_layersInstalled) return enable ? InstallThirdPersonLayers() : false;
            if (TpState::g_animationsActive == enable) return true;

            auto* model = GetAnimatedModel();
            if (model->GetDefaultLayerCount() != kTotalCount)
            {
                Log("TpLayers: layer count is %d, refusing to swap",
                    model->GetDefaultLayerCount());
                return false;
            }

            // LogLayerState(enable ? "before FP->TP" : "before TP->FP");

            // whichever TP file sits at slot 0 is the one parked in the spare slot
            static_assert(kTpPlan[0].src == Src::Tp, "spare slot parks a TP layer");
            constexpr int kSpareFile = kTpPlan[0].index;

            if (!enable) // TP -> FP
            {
                for (int i = 0; i < kTotalCount; ++i)
                {
                    const auto& p = kTpPlan[i];
                    MoveOutOfSlot(i, p.src == Src::Tp ? &g_tpParked[p.index] : &g_fpParked[p.index]);
                }

                for (int i = 0; i < kSwapCount; ++i)
                    MoveIntoSlot(i, &g_fpParked[i]);

                MoveIntoSlot(kSpareSlot, &g_tpParked[kSpareFile]); // keeps count at 8

                RebuildRuleSystems();
                ApplyFpLayerFlags();
            }
            else // FP -> TP
            {
                MoveOutOfSlot(kSpareSlot, &g_tpParked[kSpareFile]);

                for (int i = 0; i < kSwapCount; ++i)
                    MoveOutOfSlot(i, &g_fpParked[i]);

                ApplyPlan(g_tpParked);

                RebuildRuleSystems();
                ApplyTpLayerFlags();
            }

            TpState::g_animationsActive = enable;
            // LogLayerState(enable ? "after FP->TP" : "after TP->FP");
            return true;
        }

    public:
        [[nodiscard]] static bool IsThirdPersonInstalled() { return TpState::g_layersInstalled; }
        [[nodiscard]] static bool IsThirdPersonActive() { return TpState::g_animationsActive; }

        void SetThirdPerson(bool enable)
        {
            SetThirdPersonAnimations(enable);
            SetThirdPersonBodyVisible(enable);
            ThirdPersonCamera::Set(enable);
        }
    };
    static_assert(offsetof(CCharacter, m_animatedModel) == 0x1270);
    static_assert(offsetof(CCharacter, m_Blackboard) == 0x33B0);
    static_assert(offsetof(CCharacter, m_pfxInstance) == 0x33A0);
    static_assert(offsetof(CCharacter, m_worldMatrix) == 0x3AA8);
    static_assert(offsetof(CCharacter, m_animalComponent) == 0x4388);
} // namespace gz
#pragma pack(pop)