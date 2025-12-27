#pragma once

#include "singleton.h"

#include <d3d11.h>

#include <CFW1StateSaver.h>
#include <FW1FontWrapper.h>

#include <string>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace gz
{
class HDevice_t;
};

/// <summary>
/// Graphics system for rendering text overlays in Generation Zero.
///
/// Unlike JC4, GZ requires manual render target setup because the game
/// unbinds the backbuffer before our hook executes. We must:
/// 1. Get fresh device/context/swapchain each frame (they're not stable)
/// 2. Manually create and set the backbuffer as render target
/// 3. Cache the RTV to avoid recreating it every frame
/// </summary>
class Graphics : public Singleton<Graphics>
{
private:
    bool m_imguiInitialized = false;
public:
    bool                    m_ready  = false;           // True when font wrapper is initialized
    ID3D11Device*           m_device = nullptr;         // D3D11 device (cached, AddRef'd)
    ID3D11DeviceContext*    m_deviceContext = nullptr;  // D3D11 device context (cached)
    IDXGISwapChain*         m_swapChain = nullptr;      // Swapchain (cached, AddRef'd)
    uint32_t                m_screenWidth = 0;          // Screen width in pixels
    uint32_t                m_screenHeight = 0;         // Screen height in pixels
    IFW1FontWrapper*        m_font   = nullptr;         // DirectWrite font renderer
    ID3D11RenderTargetView* m_cachedBackBufferRTV = nullptr;    // Cached backbuffer RTV (optimization)

    /// <summary>
    /// Check if we need to reinitialize due to device/swapchain change.
    /// This happens during fullscreen transitions or scene changes.
    /// </summary>
    bool NeedsReinit(ID3D11Device* currentDevice, IDXGISwapChain* currentSwapChain) const {
        return !m_ready || m_device != currentDevice || m_swapChain != currentSwapChain;
    }

public:
    /// <summary>
    /// Hook function called by the game's Present/Flip function.
    /// This is where we inject our rendering.
    /// </summary>
    static void GraphicsFlipCallback(void* param_1);
    static inline decltype(GraphicsFlipCallback)* pfn_Flip = nullptr;

public:
    Graphics()  = default;
    ~Graphics() = default;

    /// <summary>
    /// Clean up all D3D11 resources. Called on DLL unload.
    /// </summary>
    void Shutdown();

    /// <summary>
    /// Initialize the graphics system with a device and swapchain.
    /// Creates the font wrapper and caches screen resolution.
    /// </summary>
    void Initialize(ID3D11Device* device, IDXGISwapChain* swapChain, ID3D11DeviceContext* context);

    /// <summary>
    /// Draw a text string at normalized screen coordinates (0-1 range).
    /// </summary>
    /// <param name="context">D3D11 device context (must be fresh from current frame)</param>
    /// <param name="str">Text to draw</param>
    /// <param name="x">X position (0.0 = left, 1.0 = right)</param>
    /// <param name="y">Y position (0.0 = top, 1.0 = bottom)</param>
    /// <param name="size">Font size as fraction of screen height</param>
    /// <param name="color">ARGB color (0xAARRGGBB)</param>
    void DrawString(ID3D11DeviceContext* context, const std::string& str, float x, float y, float size, uint32_t color);
};
