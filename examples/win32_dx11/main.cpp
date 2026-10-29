#include "iwa/directx11.hpp"
#include "iwa/windows.hpp"
#include "math.hpp"
#include "utils/logger.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <d3d11.h>
#include <d3d11sdklayers.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <chrono>
#include <format>
#include <print>
#include <span>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include <windef.h>
#include <windows.h>

HINSTANCE HInstance = NULL;
HWND Hwnd = NULL;
D3D_DRIVER_TYPE D3DDriverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL D3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;
auto MainRenderer = Iwa::Renderer::New("main");
auto MainRenderContext = Iwa::RenderContext::New(MainRenderer, "main_ctx");

UINT gWidth = 0;
UINT gHeight = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HRESULT InitWindow(int nCmdShow)
{
    SetProcessDPIAware();

    const wchar_t windowName[] = L"Window Class";

    WNDCLASSW wnd;
    wnd.cbClsExtra = 0;
    wnd.cbWndExtra = 0;
    wnd.hCursor = LoadCursor(0, IDC_ARROW);
    wnd.hIcon = LoadIcon(0, IDI_WINLOGO);
    wnd.lpszMenuName = 0;
    wnd.style = 0;
    wnd.hbrBackground = 0;
    wnd.lpfnWndProc = WndProc;
    wnd.hInstance = HInstance;
    wnd.lpszClassName = windowName;

    // Register window class
    RegisterClassW(&wnd);

    // Create window
    Hwnd = CreateWindowExW(0, windowName, L"iwagui", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                             CW_USEDEFAULT, NULL, NULL, HInstance, NULL);


    if (!Hwnd)
        return E_FAIL;

    ShowWindow(Hwnd, nCmdShow);
    return S_OK;
}

HRESULT InitDevice()
{
    Iwa::LogDebug("Initializing device");
    Iwa::LogPushTab();
    HRESULT result = S_OK;
    RECT rc;

    GetClientRect(Hwnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    UINT device_flags = 0;

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);
    D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT feature_levels_count = ARRAYSIZE(feature_levels);

    DXGI_SWAP_CHAIN_DESC swap_chain_desc;                           // description of swapchain
    ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));                       // clearing because some bytes
    swap_chain_desc.BufferCount = 1;                                // one buffer because one swapchain
    swap_chain_desc.BufferDesc.Width = width;                       // width
    swap_chain_desc.BufferDesc.Height = height;                     // height
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // pixel format
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = 240;         // refreshing rate
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = Hwnd;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.Windowed = TRUE;

    auto& backend_data = MainRenderer->backend_data;
    backend_data.hwnd = Hwnd;
    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        D3DDriverType = driverTypes[driverTypeIndex];
        result = D3D11CreateDeviceAndSwapChain(nullptr, D3DDriverType, nullptr, device_flags, feature_levels,
                                                   feature_levels_count, D3D11_SDK_VERSION, &swap_chain_desc, &backend_data.swap_chain,
                                                   &backend_data.device, &D3DFeatureLevel, &backend_data.device_ctx);

        if (SUCCEEDED(result))
        {
            Iwa::LogDebug("Created device and swap chain, driver type: {}", static_cast<uint64_t>(D3DDriverType));
            break;
        }
    }

    if (FAILED(result))
    {
        Iwa::LogFatal("Couldn't create device and swapchain, code: {:#x}", result);
        return result;
    }


    ID3D11Texture2D* back_buffer = NULL;
    result = backend_data.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer);
    if (FAILED(result))
    {
        Iwa::LogFatal("Couldn't get back buffer, code: {:#x}", result);
        return result;
    }

    result = backend_data.device->CreateRenderTargetView(back_buffer, NULL, &backend_data.render_target_view);
    back_buffer->Release();

    if (FAILED(result))
    {
        Iwa::LogFatal("Couldn't create render target view, code: {:#x}", result);
        return result;
    }

    backend_data.device_ctx->OMSetRenderTargets(1, &backend_data.render_target_view, nullptr);

    // Viewport
    backend_data.viewport.Width = (FLOAT)width;
    backend_data.viewport.Height = (FLOAT)height;
    backend_data.viewport.MinDepth = 0.0f;
    backend_data.viewport.MaxDepth = 1.0f;
    backend_data.viewport.TopLeftX = 0;
    backend_data.viewport.TopLeftY = 0;

    backend_data.device_ctx->RSSetViewports(1, &backend_data.viewport);
    Iwa::LogPopTab();

    return S_OK;
}

void CleanupDevice()
{
    MainRenderContext->Destroy();
    MainRenderer->Destroy();
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpCmdLine, int nCmdShow)
{
    HInstance = hInstance;

    HRESULT hr = InitWindow(nCmdShow);
    if (FAILED(hr))
    {
        Iwa::LogFatal("Failed InitWindow() {:#x}", hr);
        return 1;
    }
    else Iwa::LogInfo("InitWindow()");

    hr = InitDevice();
    if (FAILED(hr))
    {
        Iwa::LogFatal("Failed InitDevice() {:#x}", hr);
        CleanupDevice();
        return 1;
    }
    else Iwa::LogInfo("InitDevice()");

    if (MainRenderer->Initialize())
    {
        Iwa::LogFatal("Failed MainRenderer->Initialize()");
        CleanupDevice();
        return 0;
    }
    else Iwa::LogInfo("MainRenderer->Initialize()");

    MainRenderer->AddRenderContext(MainRenderContext);

    std::vector<uint8_t> arial_font_data = {
        #embed "res/arial.TTF"
    };

    std::vector<uint8_t> verdanai_font_data = {
        #embed "res/verdanai.TTF"
    };

    std::array<Iwa::CharRange_t, 2> ranges = {
        Iwa::CharRange_t::Cyrillic(),
        Iwa::CharRange_t::ASCII(),
    };

    auto arial_font = Iwa::BitmapFont::New(MainRenderer, "test_font", 55)->Load(arial_font_data, ranges);
    auto verdanai_font = Iwa::BitmapFont::New(MainRenderer, "test_font2", 32)->Load(verdanai_font_data, ranges);

    MSG msg = {};

    std::unordered_set<UINT> exit_codes = { WM_QUIT, WM_CLOSE, WM_DESTROY, WM_NCDESTROY };
    while (!exit_codes.contains(msg.message))
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        MainRenderer->PreRender();
        MainRenderContext->RenderWText({0,80}, *arial_font, L"Привет Hello", 0xFFFFFFFF);
        MainRenderContext->RenderWText({0,120}, *verdanai_font, L"Привет Hello", 0xFFFFFFFF);
        MainRenderContext->RenderWText({0,140}, *verdanai_font, L"Привет Hello", 0xFFFFFFFF);
        MainRenderer->Render();

        // technologies
        Rectpx_t t = {0_px, 0_px, 100_px, 100_px};
        Iwa::LogInfo("{}", t.getArea());
    }


    arial_font->Destroy();
    verdanai_font->Destroy();
    CleanupDevice();

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (Iwa::WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    switch (uMsg)
    {
    case WM_PAINT:
        break;
    case WM_SIZE:
    {
        gWidth = (UINT)LOWORD(lParam);
        gHeight = (UINT)HIWORD(lParam);

        MainRenderer->ResizeViewport(Vec2_t(gWidth, gHeight));

        return false;
    }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    return false;
}
