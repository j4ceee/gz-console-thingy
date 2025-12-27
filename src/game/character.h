#pragma once

#pragma pack(push, 1)
#include <cmath>

namespace gz
{
class CCharacter
{
  public:
    char      _pad[0x208];
    int       m_maxHealth;
    char      _pad2[0x80];
    int       m_health;

    /*
    char               _pad[0x114];
    CMatrix4f          m_worldTransform; // 114 - 154
    char               _pad4[0x460];
    bool               m_unlimitedAmmo; // 2288 */

    int GetHealth() const { return m_health; }
    void SetHealth(int health) { m_health = health; }
    int GetMaxHealth() const { return m_maxHealth; }
    void SetMaxHealth(int maxHealth) { m_maxHealth = maxHealth; }

    /// <summary>
    /// Gets the health of the character as a percentage (0-100).
    /// </summary>
    /// @return Health percentage as an integer.
    int GetHealthInPercentage() const
    {
        if (m_maxHealth == 0) {
            return 0;
        }
        return static_cast<int>(std::round((static_cast<float>(m_health) / static_cast<float>(m_maxHealth)) * 100.0f));
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
}; // namespace gz
#pragma pack(pop)
