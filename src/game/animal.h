#pragma once

#include <algorithm>

#include "animal_character.h"
#include "animal_population.h"
#include "animal_type.h"
#include "data_types.h"
#include "meow_hook/util.h"
#include "character.h"

#pragma pack(push, 1)
namespace gz
{
    class CSpawnedAnimalNetworkComponent;

    class CAnimalHealth
    {
    public:
        void*       vtable;                 // 0x00 → 0x08
        char        _padHits[0x18];         // 0x08 → 0x20
        char        _padPartHealth[0x18];   // 0x20 → 0x38
        float       m_MaxOrganGroupHealth;  // 0x38 → 0x3C
        float       m_OrganGroupHealth;     // 0x3C → 0x40
        float       m_MinKillDamage;        // 0x40 → 0x44
        char        _pad2;                  // 0x44 → 0x45
        bool        m_IsInvincible;         // 0x45 → 0x46
        char        _pad3[0x3A];            // 0x46 → 0x80
        CAnimal*    m_Animal;               // 0x80 → 0x88

        [[nodiscard]] float GetHealthPercentByLethalDamage() const
        {
            if (m_MinKillDamage <= 0.0f) return 0.0f;
            const float hp = (m_OrganGroupHealth - (m_MaxOrganGroupHealth - m_MinKillDamage)) / m_MinKillDamage;
            return std::clamp(hp * 100.0f, 0.0f, 100.0f);
        }

        [[nodiscard]] int GetHealthPercentByMaxHealth() const
        {
            return static_cast<int>(std::round(
                (static_cast<float>(m_OrganGroupHealth) / static_cast<float>(m_MaxOrganGroupHealth)) * 100.0f));
        }

        void SetHealthInPercentageByMaxHealth(int percentage)
        {
            if (percentage < 0) percentage = 0;
            if (percentage > 100) percentage = 100;

            // calculate the new health based on the percentage of max health
            const float newHealth = (static_cast<float>(percentage) / 100.0f) * m_MaxOrganGroupHealth;
            // calculate the damage to apply (positive for damage, negative for healing)
            const float damage = m_OrganGroupHealth - newHealth;

            ApplyDamage(damage);
        }

        void SetHealthInPercentageByLethalDamage(int percentage)
        {
            if (percentage < 0) percentage = 0;
            if (percentage > 100) percentage = 100;

            // invert GetHealthPercentByLethalDamage formula
            const float newHealth = (static_cast<float>(percentage) / 100.0f)
                                    * m_MinKillDamage
                                    + (m_MaxOrganGroupHealth - m_MinKillDamage);
            const float damage = m_OrganGroupHealth - newHealth;
            ApplyDamage(damage);
        }

        // ratio for parts: newHealth relative to max, since part health sums to m_MaxOrganGroupHealth
        [[nodiscard]] float GetPartsRatioForLethalPercent(int percentage) const
        {
            const float newHealth = (static_cast<float>(percentage) / 100.0f)
                                    * m_MinKillDamage
                                    + (m_MaxOrganGroupHealth - m_MinKillDamage);
            return newHealth / m_MaxOrganGroupHealth;
        }

        /// <summary>
        /// Applies damage to the animal's health. Positive values will deal damage, negative values will heal.
        /// </summary>
        /// <param name="damage">The amount of damage to apply. Positive for damage, negative for healing.</param>
        /// <param name="silent">If true, the damage will not be synced with other clients in multiplayer.</param>
        void ApplyDamage(float damage, bool silent = false)
        {
            // clamp damage, but allow healing (negative damage)
            if (damage > 0.0f) { // taking damage
                damage = std::min(damage, m_OrganGroupHealth);
            }
            else { // healing
                // only heal as much as the current damage (= max health - current health)
                const float currentDamage = m_MaxOrganGroupHealth - m_OrganGroupHealth;
                damage = std::max(damage, -currentDamage);
            }

            meow_hook::func_call<int>(
                GetAddress(ANIMAL_HEALTH_DAMAGE),
                this,
                damage,
                silent
            );
        }
    };
    static_assert(sizeof(CAnimalHealth) == 0x88, "CAnimalHealth size mismatch"); // temp size

    class CAnimal
    {
    public:
        char                _pad0[0x38];                // 0x00 → 0x38
        uint32_t            m_NetworkComponentId;       // 0x38 → 0x3C
        char                _pad1[0x45];                // 0x3C → 0x81
        std::byte           m_Flags;                    // 0x81 → 0x82
        char                _pad2[0x0E];                // 0x82 → 0x90
        BasicSharedPtr      m_SpawnedCharacter;         // 0x90 → 0xA0
        CVector2f           m_MapPosition;              // 0xA0 → 0xA8
        CVector2f           m_MapDirection;             // 0xA8 → 0xB0
        char                _pad3[0x04];                // 0xB0 → 0xB4
        float               m_ScriptedSpawnPositionY;   // 0xB4 → 0xB8
        char                _pad4[0x8];                 // 0xB8 → 0xC0
        CAnimalType*        m_AnimalType;               // 0xC0 → 0xC8
        CAnimalPopulation*  m_AnimalPopulation;         // 0xC8 → 0xD0
        std::byte           m_Loadout;                  // 0xD0 → 0xD1
        char                _pad5[0x07];                // 0xD1 → 0xD8
        CAnimalGroup*       m_Group;                    // 0xD8 → 0xE0
        CAnimalHealth       m_Health;                   // 0xE0 → 0x168 (at current size of 0x88)
        char                _pad6[0x78];                // 0x168 → 0x1E0
        uint32_t            m_VisualVariationSeed;      // 0x1E0 → 0x1E4

        [[nodiscard]] CCharacter* GetSpawnedCharacter() const
        {
            return *reinterpret_cast<CCharacter**>(
                const_cast<char*>(reinterpret_cast<const char*>(&m_SpawnedCharacter))
            );
        }

        [[nodiscard]] CAnimalCharacterComponent* GetSpawnedCharacterComponent() const
        {
            if (auto* spawnedChar = GetSpawnedCharacter())
            {
                return spawnedChar->GetAnimalComponent();
            }
            return nullptr;
        }

        [[nodiscard]] CAnimalType* GetAnimalType() const
        {
            return m_AnimalType;
        }

        CAnimalHealth* GetHealth()
        {
            return &m_Health;
        }

        [[nodiscard]] float GetHealthPercentByLethalDamage() const
        {
            return m_Health.GetHealthPercentByLethalDamage();
        }

        [[nodiscard]] int GetHealthPercentByMaxHealth() const
        {
            return m_Health.GetHealthPercentByMaxHealth();
        }

        /// <summary>
        /// Sets the machine's health based on a percentage of its max health. For example, 50% will set the health to half of the max health.
        /// </summary>
        /// TODO: this still is not safe: may not be synced properly in multiplayer + SetHealthRatioOnParts() only works on parts that are not already destroyed
        void SetHealthInPercentageByLethalDamage(int percentage)
        {
            m_Health.SetHealthInPercentageByLethalDamage(percentage);
            const auto partsRatio = m_Health.GetPartsRatioForLethalPercent(percentage);

            if (auto* animalComp = GetSpawnedCharacterComponent())
            {
                animalComp->SetHealthRatioOnParts(partsRatio);
            }
        }

        // currently unused
        void* CreateNetworkComponent()
        {
            auto* mgr = *(void**)GetAddress(INST_NETWORK_COMP_MANAGER);

            void* component = meow_hook::func_call<void*>(
                GetAddress(NETWORK_COMP_CREATE),
                mgr,
                (uint32_t)6,            // component type: animal
                (void*)nullptr,         // owner character ptr: none yet
                (uint64_t)0,            // SObjectID: default/empty
                (uint32_t)0,            // unknown, always 0 in reference
                (uint32_t)0xFFFFFFFF,   // network id: auto-assign
                (uint8_t)0xFF,          // owner peer: auto-resolve
                (uint32_t)0xFFFFFFFF    // parent NC id: none
            );

            if (!component) return nullptr;

            meow_hook::func_call<void>(GetAddress(NETWORK_COMP_SEND_ADD_EVENT), mgr, component, (uint8_t)0xFF);

            m_NetworkComponentId = *(uint32_t*)((char*)component + 0x10);
            return component;
        }

        [[nodiscard]] CSpawnedAnimalNetworkComponent* GetNetworkComponent() const
        {
            if (m_NetworkComponentId == 0xFFFFFFFF)
                return nullptr;

            auto* mgr = (void*)GetAddress(INST_NETWORK_COMP_MANAGER);
            mgr = *(void**)mgr;

            auto* comp = meow_hook::func_call<CSpawnedAnimalNetworkComponent*>(
                GetAddress(FIND_NETWORK_COMPONENT_CHILDREN),
                mgr,
                m_NetworkComponentId
            );

            return comp;
        }
    };
    static_assert(offsetof(CAnimal, m_NetworkComponentId) == 0x38);
    static_assert(offsetof(CAnimal, m_Flags) == 0x81);
    static_assert(offsetof(CAnimal, m_SpawnedCharacter) == 0x90);
    static_assert(offsetof(CAnimal, m_AnimalType) == 0xC0);
    static_assert(offsetof(CAnimal, m_AnimalPopulation) == 0xC8);
    static_assert(offsetof(CAnimal, m_Group) == 0xD8);
    static_assert(offsetof(CAnimal, m_Health) == 0xE0);

} // namespace gz
#pragma pack(pop)