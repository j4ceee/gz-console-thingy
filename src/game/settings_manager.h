#pragma once

#include "addresses.h"
#include "meow_hook/util.h"

#pragma pack(push, 1)
namespace gz
{
    /// keys for CSettingsManager::GetValue
    enum class ESettingId : int
    {
        GameDisplayUnits = 0x3b, // 0 = metric (km/h), 1 = imperial (mph), anything else = m/s
    };

    class CSettingsManager
    {
    public:
        static CSettingsManager* instance()
        {
            return *(CSettingsManager**)GetAddress(INST_SETTINGS_MANAGER);
        }

        [[nodiscard]] int GetValue(ESettingId id) const
        {
            return meow_hook::func_call<int>(GetAddress(SETTINGS_GET_VALUE), this, static_cast<int>(id));
        }

        [[nodiscard]] int GetDisplayUnits() const
        {
            return this->GetValue(ESettingId::GameDisplayUnits);
        }
    };
} // namespace gz
#pragma pack(pop)
