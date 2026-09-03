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
        char        _pad1[0x1B6];   // 0x02 → 0x1B8
        uint64_t    m_LocalGuid;    // 0x1B8 → 0x1C0
        uint8_t     m_HostPeerID;   // 0x1C0 → 0x1C1
    };
    static_assert(offsetof(CNetworkConnection, m_LocalPeerID) == 0x01);
    static_assert(offsetof(CNetworkConnection, m_LocalGuid) == 0x1B8);
    static_assert(offsetof(CNetworkConnection, m_HostPeerID) == 0x1C0);

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

        [[nodiscard]] uint8_t GetHostPeerID() const
        {
            return m_Connection ? m_Connection->m_HostPeerID : 0;
        }

        [[nodiscard]] uint64_t GetLocalGuid() const
        {
            return m_Connection ? m_Connection->m_LocalGuid : 0;
        }

        [[nodiscard]] bool IsHost() const
        {
            return m_Connection && m_Connection->m_LocalPeerID == m_Connection->m_HostPeerID;
        }

        [[nodiscard]] bool IsHost(const uint8_t otherId) const
        {
            return m_Connection && otherId == m_Connection->m_HostPeerID;
        }
    };
    static_assert(offsetof(CBaseNetworkManager, m_Connection) == 0x10);
} // namespace gz
#pragma pack(pop)