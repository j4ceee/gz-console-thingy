#pragma once
#include <cstddef>
#include <cstdint>

#pragma pack(push, 1)
namespace gz
{
    namespace NAnimationSystem
    {
        struct CState
        {
            void* m_vtable; // +0x00
            char _pad[0x04]; // +0x08
            uint32_t m_Hash; // +0x0C
        };

        struct SStateMachineResourceInfo
        {
            void* m_Resource; // +0x00 → CStateMachine*
        };

        struct CStateMachine
        {
            char _pad0[0x38];
            CState** m_States; // +0x38 sorted array
            char _pad1[0x28];
            uint32_t m_StateCount; // +0x68
        };
        static_assert(offsetof(CStateMachine, m_States) == 0x38);
        static_assert(offsetof(CStateMachine, m_StateCount) == 0x68);

        struct CStateMachineInstance
        {
            char                        _pad0[0x10];
            SStateMachineResourceInfo*  m_StateMachineHandle; // +0x10
            CState*                     m_CurrentState; // +0x18
            char                        _pad1[0x18];
            void*                       m_CallbackUserData; // +0x30
            void*                       m_OnStateChange; // +0x38

            [[nodiscard]] CStateMachine* GetStateMachine() const
            {
                if (!m_StateMachineHandle) return nullptr;
                return static_cast<CStateMachine*>(m_StateMachineHandle->m_Resource);
            }
        };
        static_assert(offsetof(CStateMachineInstance, m_StateMachineHandle) == 0x10);
        static_assert(offsetof(CStateMachineInstance, m_CurrentState) == 0x18);
    }

    struct CAnimationRuleSystem
    {
        bool    m_Debug;                // +0x00
        char    _pad0[0x07];
        void*   m_OnTransitionCallback; // +0x08
        char    _pad1[0x10];            // +0x10
        bool    m_HasChangedState;      // +0x20
        bool    m_AnimFinished;         // +0x21
        char    _pad2[0x02];
        float   m_HeightOverGround;     // +0x24
        float   m_HeightOverGroundExcludingWater; // +0x28
        float   m_FallDistance;         // +0x2C
        float   m_TimeUntilLand;        // +0x30
        uint32_t m_PreviousStateId;     // +0x34

        // shared_ptr<CStateMachineInstance> -> px then pn, never touch pn
        NAnimationSystem::CStateMachineInstance* m_StateMachineInstance; // +0x38
        void* m_StateMachineInstance_pn; // +0x40

        [[nodiscard]] NAnimationSystem::CStateMachineInstance* GetSMI() const
        {
            return m_StateMachineInstance;
        }
    };
    static_assert(offsetof(CAnimationRuleSystem, m_PreviousStateId) == 0x34);
    static_assert(offsetof(CAnimationRuleSystem, m_StateMachineInstance) == 0x38);
    static_assert(offsetof(CAnimationRuleSystem, m_StateMachineInstance_pn) == 0x40);
} // namespace gz
#pragma pack(pop)
