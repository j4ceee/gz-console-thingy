#include "graphics.h"
#include "imgui/ui.h"
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include "input.h"
#include "log.h"
#include "util.h"

#include "game/device.h"

// graphics.cpp
void Graphics::GraphicsFlipCallback(void* param_1)
{
    const auto& gfx = Get();

    // Only retrieve fresh if not initialized yet
    if (!gfx->m_ready) {
        gz::Log("Graphics not ready, attempting to initialize...");

        ID3D11DeviceContext* context = gz::HDevice_t::GetDeviceContext(param_1);
        IDXGISwapChain* swapChain = gz::HDevice_t::GetSwapChain(param_1);

        gz::Log("GetDeviceContext returned: %p", (void*)context);
        gz::Log("GetSwapChain returned: %p", (void*)swapChain);

        if (!context || !swapChain) {
            gz::Log("Context or swapchain is null - aborting init");
            return pfn_Flip(param_1);
        }

        ID3D11Device* device = nullptr;
        context->GetDevice(&device);
        gz::Log("GetDevice returned: %p", (void*)device);

        if (!device) {
            gz::Log("Device is null - aborting init");
            return pfn_Flip(param_1);
        }

        gfx->Initialize(device, swapChain, context);
        device->Release();
    }

    // Use cached context from now on
    if (gfx->m_ready && gfx->m_deviceContext) {
        // Only create RTV if we don't have one cached
        if (!gfx->m_cachedBackBufferRTV) {
            gz::Log("Creating cached backbuffer RTV...");
            ID3D11Texture2D* backBuffer = nullptr;
            HRESULT hr = gfx->m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
            if (SUCCEEDED(hr)) {
                hr = gfx->m_device->CreateRenderTargetView(backBuffer, nullptr, &gfx->m_cachedBackBufferRTV);
                gz::Log("CreateRenderTargetView result: 0x%X, RTV=%p", hr, (void*)gfx->m_cachedBackBufferRTV);
                backBuffer->Release();
            } else {
                gz::Log("GetBuffer failed: 0x%X", hr);
            }
        }

        if (gfx->m_cachedBackBufferRTV) {
            // Save the game's current render target and viewport
            ID3D11RenderTargetView* oldRTV = nullptr;
            ID3D11DepthStencilView* oldDSV = nullptr;
            D3D11_VIEWPORT oldViewport;
            UINT numViewports = 1;

            gfx->m_deviceContext->OMGetRenderTargets(1, &oldRTV, &oldDSV);
            gfx->m_deviceContext->RSGetViewports(&numViewports, &oldViewport);

            // Set our backbuffer as the render target
            gfx->m_deviceContext->OMSetRenderTargets(1, &gfx->m_cachedBackBufferRTV, nullptr);

            // Set viewport to full screen
            D3D11_VIEWPORT viewport;
            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            viewport.Width = (FLOAT)gfx->m_screenWidth;
            viewport.Height = (FLOAT)gfx->m_screenHeight;
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            gfx->m_deviceContext->RSSetViewports(1, &viewport);

            // update ImGui
            if (gfx->m_imguiInitialized) {
                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                // always call Render - it handles visibility internally
                gz::UI::Get()->Render();

                ImGui::Render();

                // always render draw data if we have any
                ImDrawData* drawData = ImGui::GetDrawData();
                if (drawData && drawData->TotalVtxCount > 0) {
                    ImGui_ImplDX11_RenderDrawData(drawData);
                }
            }

            // draw our overlay
            Input::Get()->Draw(gfx->m_deviceContext);

            // restore the game's render target and viewport
            gfx->m_deviceContext->OMSetRenderTargets(1, &oldRTV, oldDSV);
            gfx->m_deviceContext->RSSetViewports(1, &oldViewport);

            if (oldRTV) oldRTV->Release();
            if (oldDSV) oldDSV->Release();
        }
    }

    return pfn_Flip(param_1);
}

void Graphics::Initialize(ID3D11Device* device, IDXGISwapChain* swapChain, ID3D11DeviceContext* context)
{
    if (!device || !swapChain || !context) {
        gz::Log("Initialize called with null parameters!");
        return;
    }

    // store references to device and swapchain
    m_device = device;
    m_device->AddRef();

    m_swapChain = swapChain;
    m_swapChain->AddRef();

    m_deviceContext = context;
    m_deviceContext->AddRef();

    // Get and cache screen resolution
    DXGI_SWAP_CHAIN_DESC desc;
    if (SUCCEEDED(swapChain->GetDesc(&desc))) {
        m_screenWidth = desc.BufferDesc.Width;
        m_screenHeight = desc.BufferDesc.Height;
        gz::Log("Screen resolution: %dx%d", m_screenWidth, m_screenHeight);
    } else {
        gz::Log("Failed to get swapchain desc!");
        return;
    }

    // create FW1FontWrapper for text rendering (optional - ImGui works without it)
    IFW1Factory* factory = nullptr;
    auto hr = FW1CreateFactory(FW1_VERSION, &factory);
    gz::Log("FW1CreateFactory result: 0x%X", hr);

    if (SUCCEEDED(hr)) {
        FW1_FONTWRAPPERCREATEPARAMS createParams{};
        createParams.GlyphSheetWidth = 512;
        createParams.GlyphSheetHeight = 512;
        createParams.MaxGlyphCountPerSheet = 2048;
        createParams.SheetMipLevels = 1;
        createParams.AnisotropicFiltering = FALSE;
        createParams.MaxGlyphWidth = 384;
        createParams.MaxGlyphHeight = 384;
        createParams.DisableGeometryShader = FALSE;
        createParams.VertexBufferSize = 0;
        createParams.DefaultFontParams.pszFontFamily = L"Arial";
        createParams.DefaultFontParams.FontWeight = DWRITE_FONT_WEIGHT_SEMI_BOLD;
        createParams.DefaultFontParams.FontStyle = DWRITE_FONT_STYLE_NORMAL;
        createParams.DefaultFontParams.FontStretch = DWRITE_FONT_STRETCH_NORMAL;
        createParams.DefaultFontParams.pszLocale = L"";

        hr = factory->CreateFontWrapper(m_device, nullptr, &createParams, &m_font);
        gz::Log("CreateFontWrapper result: 0x%X", hr);
        if (SUCCEEDED(hr)) {
            gz::Log("Font wrapper created successfully!");
        } else {
            gz::Log("Failed to create font wrapper - hint text will use ImGui instead");
        }

        factory->Release();
    } else {
        gz::Log("Failed to create FW1Factory - hint text will use ImGui instead");
    }

    if (!m_imguiInitialized) {
        gz::Log("Initializing ImGui...");
        gz::UI::Get()->Initialize();

        if (SUCCEEDED(swapChain->GetDesc(&desc))) {
            ImGui_ImplWin32_Init(desc.OutputWindow);
            ImGui_ImplDX11_Init(m_device, m_deviceContext);
            m_imguiInitialized = true;
            gz::Log("ImGui backends initialized!");
            m_ready = true; // mark graphics as ready (font wrapper is not required)
        } else {
            gz::Log("Failed to get swap chain desc for ImGui!");
        }
    }
}

void Graphics::Shutdown()
{
    m_ready = false;

    // shutdown ImGui if initialized
    if (m_imguiInitialized) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        gz::UI::Get()->Shutdown();
        m_imguiInitialized = false;
    }

    // Release all COM objects in reverse order of creation
    if (m_cachedBackBufferRTV) {
        m_cachedBackBufferRTV->Release();
        m_cachedBackBufferRTV = nullptr;
    }

    if (m_font) {
        m_font->Release();
        m_font = nullptr;
    }

    if (m_deviceContext) {
        m_deviceContext->Release();
        m_deviceContext = nullptr;
    }

    if (m_swapChain) {
        m_swapChain->Release();
        m_swapChain = nullptr;
    }

    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }
}

void Graphics::DrawString(ID3D11DeviceContext* context, const std::string& str, float x, float y, float size, uint32_t color)
{
    if (!m_ready || !context || !m_font) {
        return;
    }

    // Convert normalized coordinates (0-1) to pixel coordinates
    x = (x * m_screenWidth);
    y = (y * m_screenHeight);
    size = (size * m_screenHeight); // Size is relative to screen height

    m_font->DrawString(context, util::ToWideString(str).c_str(), size, x, y, color, 0);
}