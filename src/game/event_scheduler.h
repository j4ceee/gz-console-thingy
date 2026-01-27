#pragma once
#include <cstdint>
#include <vector>
#include "addresses.h"
#include "hook_helpers.h"
#include "patches/event_time_patch.h"
#include "util/hash_utils.h"

#pragma pack(push, 1)
namespace gz
{
    // known event hashes
    namespace Events {
        inline uint32_t WINTER = 0;
        inline uint32_t LUNAR_NEW_YEAR = 0;
        inline uint32_t SEMLA = 0;
        inline uint32_t HALLOWEEN = 0;
        inline uint32_t ANNIVERSARY = 0;

        inline void InitializeHashes() {
            WINTER = Utils::HashString("winter_2019_01");
            LUNAR_NEW_YEAR = Utils::HashString("lunar_new_year_event_active");
            SEMLA = Utils::HashString("semla_event_active");
            HALLOWEEN = Utils::HashString("halloween_2019_01");
            ANNIVERSARY = Utils::HashString("anniversary_event_active");
        }
    }

    class EventManager
    {
    public:
        uint32_t* activeEventsStart;      // +0x00
        uint32_t* activeEventsCurrent;    // +0x08
        uint32_t* activeEventsEnd;        // +0x10
        uint32_t* specialEventsStart;     // +0x18
        uint32_t* specialEventsCurrent;   // +0x20
        uint32_t* specialEventsEnd;       // +0x28
        int64_t nextUpdateTime;           // +0x30

        std::vector<uint32_t> GetActiveEvents() {
            std::vector<uint32_t> events;
            if (!activeEventsStart || !activeEventsCurrent) {
                return events;
            }
            // validate pointers before dereferencing
            size_t count = activeEventsCurrent - activeEventsStart;
            if (count > 100) return events; // sanity check
            for (size_t i = 0; i < count; i++) {
                events.push_back(activeEventsStart[i]);
            }
            return events;
        }

        bool IsEventActive(uint32_t eventHash) {
            if (!activeEventsStart || !activeEventsCurrent) {
                return false;
            }
            size_t count = activeEventsCurrent - activeEventsStart;
            if (count > 100) return false; // sanity check
            for (size_t i = 0; i < count; i++) {
                if (activeEventsStart[i] == eventHash) {
                    return true;
                }
            }
            return false;
        }

        void ClearActiveEvents() {
            EventTimePatch::RestoreOriginal();

            if (!activeEventsStart) return;

            // reset current pointer to start (empties the vector)
            activeEventsCurrent = activeEventsStart;
        }

    // -- HOOK SETUP & MANAGEMENT --

    private:
        using EventSchedulerFunc = void(__fastcall*)(EventManager* mgr);
        static inline EventSchedulerFunc s_originalEventScheduler;
        static inline bool s_blockScheduler;
        static inline bool s_requestedEventChange;

        // hooked function that captures the EventManager pointer
        static void HookedEventScheduler(EventManager* mgr) {
            // call original function if not blocked
            if ((!s_blockScheduler || s_requestedEventChange) && s_originalEventScheduler) {
                if (s_requestedEventChange) {
                    s_requestedEventChange = false;
                }
                s_originalEventScheduler(mgr);
            }
        }

        static bool SetupEventManagerHook() {
            if (s_originalEventScheduler != nullptr) {
                return true; // already hooked
            }
            return MH_CreateHookGZ(EVENT_SCHEDULER, &HookedEventScheduler, &s_originalEventScheduler);
        }

    public:
        static bool Initialize() {
            bool success = SetupEventManagerHook();
            if (success) {
                Events::InitializeHashes();
                s_blockScheduler = true;
                s_requestedEventChange = false;
            }
            return success;
        }

        static void SetCustomEvent(uint32_t eventHash) {
            time_t fakeTime = 0;

            if (eventHash == Events::WINTER) {
                fakeTime = EventTimePatch::WINTER_TIME;
            } else if (eventHash == Events::LUNAR_NEW_YEAR) {
                fakeTime = EventTimePatch::LUNAR_TIME;
            } else if (eventHash == Events::SEMLA) {
                fakeTime = EventTimePatch::SEMLA_TIME;
            } else if (eventHash == Events::ANNIVERSARY) {
                fakeTime = EventTimePatch::ANNIVERSARY_TIME;
            } else if (eventHash == Events::HALLOWEEN) {
                fakeTime = EventTimePatch::HALLOWEEN_TIME;
            }

            if (fakeTime != 0) {
                EventTimePatch::SetFakeTime(fakeTime);
                s_requestedEventChange = true;
            }
        }

        static bool IsSchedulerBlocked() {
            return s_blockScheduler;
        }

        static void SetSchedulerBlocked(bool blocked) {
            s_blockScheduler = blocked;
        }

        static const char* GetEventName(uint32_t hash) {
            if (hash == Events::WINTER) return "Winter";
            if (hash == Events::LUNAR_NEW_YEAR) return "Lunar New Year";
            if (hash == Events::SEMLA) return "Semla Event";
            if (hash == Events::HALLOWEEN) return "Halloween";
            if (hash == Events::ANNIVERSARY) return "Anniversary";
            return "Unknown Event";
        }
    };

    class CSocialManager
    {
    public:
        char            _pad[0x18];         // 0x00 → 0x18
        EventManager*   m_eventManager;     // 0x18 → 0x20

        static CSocialManager* instance()
        {
            return *(CSocialManager**)(GetAddress(INST_SOCIAL_MANAGER));
        }

        EventManager* GetEventManager()
        {
            return m_eventManager;
        }
    };
} // namespace gz
#pragma pack(pop)