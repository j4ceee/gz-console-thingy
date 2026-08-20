#pragma once

#include <cstddef>
#include <cstdint>
#include "addresses.h"

#pragma pack(push, 1)
namespace gz
{
    class CNetworkConnection
    {
    public:
        char        _pad0[0x01];    // 0x00 → 0x01
        uint8_t     m_LocalPeerID;  // 0x01 → 0x02
        char        _pad1[0xC6];    // 0x02 → 0xC8
        uint64_t    m_LocalGuid;    // 0xC8 → 0xD0
        uint8_t     m_HostPeerID;   // 0xD0 → 0xD1
    };
    static_assert(offsetof(CNetworkConnection, m_LocalPeerID) == 0x01);
    static_assert(offsetof(CNetworkConnection, m_LocalGuid) == 0xC8);
    static_assert(offsetof(CNetworkConnection, m_HostPeerID) == 0xD0);

    class CBaseNetworkManager
    {
    public:
        char                _pad0[0x10];    // 0x00 → 0x10
        CNetworkConnection* m_Connection;   // 0x10 → 0x18

        static CBaseNetworkManager* instance()
        {
            return *(CBaseNetworkManager**)GetAddress(INST_BASE_NETWORK_MANAGER);
        }

        [[nodiscard]] uint8_t GetLocalPeerID() const
        {
            return m_Connection ? m_Connection->m_LocalPeerID : 0;
        }

        [[nodiscard]] bool IsHost() const
        {
            return m_Connection && m_Connection->m_LocalPeerID == m_Connection->m_HostPeerID;
        }
    };
    static_assert(offsetof(CBaseNetworkManager, m_Connection) == 0x10);
} // namespace gz
#pragma pack(pop)