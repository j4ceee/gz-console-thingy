#pragma once
#include "patch_info.h"
#include "../log.h"

namespace gz
{
class PatchGroup
{
private:
    const char* m_name;
    const PatchInfo* m_patches;
    const size_t m_patchCount;
    bool m_enabled;
    bool m_initialized;

public:
    PatchGroup(const char* name, const PatchInfo* patches, size_t count)
        : m_name(name)
        , m_patches(patches)
        , m_patchCount(count)
        , m_enabled(false)
        , m_initialized(false)
    {}

    bool Initialize()
    {
        if (m_initialized) return true;
        m_initialized = PatchUtils::VerifyAll(m_patches, m_patchCount);
        if (!m_initialized) {
            Log("PatchGroup '%s' failed to initialize: original bytes do not match expected values.", m_name);
        } else {
            Log("PatchGroup '%s' initialized successfully.", m_name);
        }
        return m_initialized;
    }

    void Enable()
    {
        if (!m_initialized || m_enabled) return;
        PatchUtils::ApplyAll(m_patches, m_patchCount);
        m_enabled = true;
    }

    void Disable()
    {
        if (!m_initialized || !m_enabled) return;
        PatchUtils::RestoreAll(m_patches, m_patchCount);
        m_enabled = false;
    }

    void Toggle()
    {
        if (m_enabled) {
            Disable();
        } else {
            Enable();
        }
    }
    
    bool IsEnabled() const { return m_enabled; }
    bool IsInitialized() const { return m_initialized; }
    const char* GetName() const { return m_name; }
};
} // namespace gz