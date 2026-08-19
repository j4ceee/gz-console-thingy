#pragma once

#include "data_types.h"

#pragma pack(push, 1)
namespace gz
{
    class CPoseProducer
    {
    public:

    };

    class CAnimationControl
    {
    public:
        char            _pad[0xA0];     // 0x00 → 0xA0
        CPoseProducer*  m_poseProducer; // 0xA0

        [[nodiscard]] CPoseProducer* GetPoseProducer()
        {
            return m_poseProducer;
        }
    };

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

        [[nodiscard]] CAnimationControl* GetAnimationControl() const
        {
            return m_AnimationController;
        }
    };
    static_assert(sizeof(CAnimatedModel) == 0x150);
    static_assert(offsetof(CAnimatedModel, m_RuleSystems) == 0x80);
    static_assert(offsetof(CAnimatedModel, m_AnimationController) == 0x120);
} // namespace gz
#pragma pack(pop)
