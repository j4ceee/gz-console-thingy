#pragma once

#include "data_types.h"

#pragma pack(push, 1)
namespace gz
{
    struct SAnimationLayerInfo
    {
        GameString  m_AfsmFileName; // +0x00
        uint32_t    m_AfsmFileHash; // +0x20
        char        _pad0[4];
        GameString  m_AsFileName;   // +0x28
        uint32_t    m_AsFileHash;   // +0x48
        uint32_t    m_LayerHash;    // +0x4C (arbitrary unique id for this layer)
        int32_t     m_LayerIndex;   // +0x50 (target slot index)
        char        _pad1[4];
    };
    static_assert(sizeof(SAnimationLayerInfo) == 0x58);
    static_assert(offsetof(SAnimationLayerInfo, m_AfsmFileHash) == 0x20);
    static_assert(offsetof(SAnimationLayerInfo, m_AsFileName) == 0x28);
    static_assert(offsetof(SAnimationLayerInfo, m_AsFileHash) == 0x48);
    static_assert(offsetof(SAnimationLayerInfo, m_LayerHash) == 0x4C);
    static_assert(offsetof(SAnimationLayerInfo, m_LayerIndex) == 0x50);


    struct SAnimationLayerInstance
    {
        BasicSharedPtr  m_StateMachine;     // +0x00
        void*           m_AnimSetHandle;    // +0x10
        uint32_t        m_AfsmFileHash;     // +0x18
        uint32_t        m_AsFileHash;       // +0x1C
        uint32_t        m_LayerHash;        // +0x20 name hash (CCharacter::s_LayerNames)
        int32_t         m_LayerIndex;       // +0x24 slot this instance belongs to

        [[nodiscard]] bool IsResolved() const
        {
            return m_StateMachine.px != nullptr && m_AnimSetHandle != nullptr;
        }
    };
    static_assert(sizeof(SAnimationLayerInstance) == 0x28);
    static_assert(offsetof(SAnimationLayerInstance, m_LayerIndex) == 0x24);


    struct SLayerHashes
    {
        uint32_t m_AfsmFileHash; // +0x00
        uint32_t m_AsFileHash; // +0x04
    };
    static_assert(sizeof(SLayerHashes) == 0x8);


    struct SStateTaskInstanceMem
    {
        void* m_Memory; // +0x00
        uint32_t m_Size; // +0x08
        uint32_t _pad; // +0x0C
    };
    static_assert(sizeof(SStateTaskInstanceMem) == 0x10);


    class CAnimationDriverTransition
    {
    public:
        char _pad0[0xA8]; // 0x00 → 0xA8
        void* m_Blender;// 0xA8
    };
    static_assert(offsetof(CAnimationDriverTransition, m_Blender) == 0xA8);


    class CAnimatedBodyPart
    {
    public:
        char _pad0[0x18]; // 0x00 → 0x18
        CAnimationDriverTransition* m_Driver; // 0x18
    };
    static_assert(offsetof(CAnimatedBodyPart, m_Driver) == 0x18);


    class CPoseProducer
    {
    public:

    };


    class CAnimationControl
    {
    public:
        char            _pad0[0xA0];            // 0x00 → 0xA0
        CPoseProducer*  m_poseProducer;         // 0xA0
        char            _pad1[0x08];            // 0xA0 → 0xB0
        BasicVector     m_BodyParts;            // 0xB0
        bool            m_InactiveBodyParts[8]; // 0xC8

        [[nodiscard]] CPoseProducer* GetPoseProducer()
        {
            return m_poseProducer;
        }

        [[nodiscard]] int GetBodyPartCount() const
        {
            return m_BodyParts.Count<CAnimatedBodyPart*>();
        }

        [[nodiscard]] CAnimatedBodyPart* GetBodyPart(int index) const
        {
            auto** slot = m_BodyParts.At<CAnimatedBodyPart*>(index);
            return slot ? *slot : nullptr;
        }

        [[nodiscard]] void* GetBlender(int index) const
        {
            auto* bodyPart = GetBodyPart(index);
            if (!bodyPart || !bodyPart->m_Driver) return nullptr;
            return bodyPart->m_Driver->m_Blender;
        }
    };
    static_assert(sizeof(CAnimationControl) == 0xD0);
    static_assert(offsetof(CAnimationControl, m_BodyParts) == 0xB0);
    static_assert(offsetof(CAnimationControl, m_InactiveBodyParts) == 0xC8);


    class CAnimatedModel
    {
    public:
        bool        m_InactiveLayers[8];    // +0x00 (8 bytes)
        void*       m_ModelInstance;        // +0x08
        CMatrix4f   m_GraphicsMatrix;       // +0x10 (64 bytes)
        BasicVector m_AnimationSets;        // +0x50 (24 bytes)
        BasicVector m_StateMachines;        // +0x68 (24 bytes)
        BasicVector m_RuleSystems;          // +0x80 (24 bytes)
        BasicVector m_LayerInfos;           // +0x98 (24 bytes)
        BasicVector m_CurrentLayerHashes;   // +0xB0 (24 bytes)
        BasicVector m_DefaultLayers;        // +0xC8 (24 bytes)
        BasicVector m_ExternalLayers;       // +0xE0 (24 bytes)
        char        _pad_map[0x10];         // +0xF8 (m_LayerNameToIndex, std::map, 16 bytes)
        BasicVector m_StateTaskMemory;      // +0x108 (24 bytes)
        CAnimationControl* m_AnimationController;   // +0x120
        void*       m_InstanceData;         // +0x128
        char        m_QueuedAnimationEvents[0x20];  // +0x130 (TArray, 32 bytes)

        static constexpr size_t kLayerInstanceSize = 0x28;

        [[nodiscard]] CAnimationControl* GetAnimationControl() const
        {
            return m_AnimationController;
        }

        [[nodiscard]] int GetDefaultLayerCount() const
        {
            return m_DefaultLayers.Count<SAnimationLayerInstance>();
        }

        [[nodiscard]] int GetExternalLayerCount() const
        {
            return m_ExternalLayers.Count<SAnimationLayerInstance>();
        }

        [[nodiscard]] int GetRuleSystemCount() const
        {
            return m_RuleSystems.Count<BasicSharedPtr>();
        }

        [[nodiscard]] int GetAnimationSetCount() const
        {
            return m_AnimationSets.Count<void*>();
        }

        [[nodiscard]] int GetCurrentLayerHashCount() const
        {
            return m_CurrentLayerHashes.Count<SLayerHashes>();
        }

        [[nodiscard]] int GetStateTaskMemoryCount() const
        {
            return m_StateTaskMemory.Count<SStateTaskInstanceMem>();
        }

        [[nodiscard]] SAnimationLayerInstance* GetDefaultLayerSlot(int i) const
        {
            return m_DefaultLayers.At<SAnimationLayerInstance>(i);
        }

        [[nodiscard]] SAnimationLayerInstance* GetExternalLayerSlot(int i) const
        {
            return m_ExternalLayers.At<SAnimationLayerInstance>(i);
        }

        [[nodiscard]] SLayerHashes* GetCurrentLayerInfo(int i) const
        {
            return m_CurrentLayerHashes.At<SLayerHashes>(i);
        }

        [[nodiscard]] SStateTaskInstanceMem* GetStateTaskMemory(int i) const
        {
            return m_StateTaskMemory.At<SStateTaskInstanceMem>(i);
        }

        [[nodiscard]] void* GetAnimationSetHandle(int i) const
        {
            auto** h = m_AnimationSets.At<void*>(i);
            return h ? *h : nullptr;
        }

        [[nodiscard]] void* GetRuleSystem(int i) const
        {
            auto* p = m_RuleSystems.At<BasicSharedPtr>(i);
            return p ? p->px : nullptr;
        }

        void SetCurrentLayerInfo(int index, uint32_t afsmHash, uint32_t asHash) const
        {
            if (auto* h = GetCurrentLayerInfo(index))
            {
                h->m_AfsmFileHash = afsmHash;
                h->m_AsFileHash = asHash;
            }
        }
    };
    static_assert(sizeof(CAnimatedModel) == 0x150);
    static_assert(offsetof(CAnimatedModel, m_RuleSystems) == 0x80);
    static_assert(offsetof(CAnimatedModel, m_AnimationController) == 0x120);
} // namespace gz
#pragma pack(pop)
