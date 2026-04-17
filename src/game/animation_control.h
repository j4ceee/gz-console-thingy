#pragma once

#include "addresses.h"
#include "meow_hook/util.h"

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
        char                _pad[0x120];        // 0x000 → 0x120
        CAnimationControl*  m_animController;   // 0x120 → 0x128

        [[nodiscard]] CAnimationControl* GetAnimationControl() const
        {
            return m_animController;
        }
    };
} // namespace gz
#pragma pack(pop)
