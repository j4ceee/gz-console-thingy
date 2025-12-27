#pragma once

#pragma pack(push, 1)
namespace gz
{
    class CVehicle
    {
    public:
        void*   vtable;             // 0x00
        char    _pad08[0x208];      // 0x08 → 0x210
        int     m_maxHealth;        // 0x210 (4 bytes)
        char    _pad214[0x80];      // 0x214 → 0x294
        int     m_health;           // 0x294
        char    _pad298[0x124C];    // 0x298 → 0x14E4
        int     occupancyState;     // 0x14E4

        bool IsPlayerInVehicle()
        {
            return occupancyState == 0;
        }

        bool IsDamaged() const
        {
            return m_health < m_maxHealth;
        }

        int GetHealth() const { return m_health; }
        void SetHealth(int health) { m_health = health; }
        int GetMaxHealth() const { return m_maxHealth; }
        void SetMaxHealth(int maxHealth) { m_maxHealth = maxHealth; }

        int GetHealthInPercentage() const
        {
            if (m_maxHealth == 0) {
                return 0;
            }
            return (m_health * 100) / m_maxHealth;
        }

        void SetHealthInPercentage(int percentage)
        {
            if (percentage < 0) {
                percentage = 0;
            } else if (percentage > 100) {
                percentage = 100;
            }
            m_health = static_cast<int>((static_cast<float>(percentage) / 100.0f) * static_cast<float>(m_maxHealth));
        }

    };
}// namespace gz
#pragma pack(pop)