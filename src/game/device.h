#pragma once

#include <d3d11.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

#pragma pack(push, 1)
namespace gz
{
/// <summary>
/// Internal rendering struct used by Generation Zero's engine.
/// The device context is buried at offset 0x8020.
/// </summary>
struct InternalRenderStruct
{
    char _pad[0x8020];
    ID3D11DeviceContext* m_deviceContext;
};

/// <summary>
/// Parameters passed to the flip/present function.
/// This is passed as void* param_1, but it's actually a pointer to this struct.
/// </summary>
struct RenderParams
{
    InternalRenderStruct* internalStruct;   // ptr[0]
    void*                 unknown1;         // ptr[1]
    void*                 unknown2;         // ptr[2]
    void*                 unknown3;         // ptr[3]
    IDXGISwapChain*       swapChain;        // ptr[4]
};

/// <summary>
/// Helper class for accessing Generation Zero's D3D11 objects.
/// Now much cleaner with proper struct definitions!
/// </summary>
class HDevice_t
{
public:
    /// <summary>
    /// Get the D3D11 device context from GZ's internal structures.
    /// </summary>
    static ID3D11DeviceContext* GetDeviceContext(void* base) {
        RenderParams* params = (RenderParams*)base;
        return params->internalStruct->m_deviceContext;
    }

    /// <summary>
    /// Get the DXGI swap chain from GZ's internal structures.
    /// </summary>
    static IDXGISwapChain* GetSwapChain(void* base) {
        RenderParams* params = (RenderParams*)base;
        return params->swapChain;
    }
};
}; // namespace gz
#pragma pack(pop)