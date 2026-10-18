#include "renderer.hpp"
#include "sdk/inputs.hpp"
#include "sdk/math.hpp"
#include "sdk/utils/logger.hpp"
#include "sdk/directx11.hpp"
#include "sdk/windows.hpp"
#include "singleton.hpp"


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
#include <vector>
#include <windows.h>

using namespace Iwa;

HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;
D3D_DRIVER_TYPE g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
SDK::Renderer* g_Renderer = SDK::Renderer::getInstance();

UINT gWidth = 0;
UINT gHeight = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct Vertex
{
    Iwa::Vec2_t pos;
};

HRESULT InitWindow(int nCmdShow)
{
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
    wnd.hInstance = g_hInst;
    wnd.lpszClassName = windowName;

    // Register window class
    RegisterClassW(&wnd);

    // Create window
    //! This returns NULL
    g_hWnd = CreateWindowExW(0, windowName, L"iwagui", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                             CW_USEDEFAULT, NULL, NULL, g_hInst, NULL);

    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, nCmdShow);
    return S_OK;
}

HRESULT InitDevice()
{
    HRESULT hr = S_OK;
    RECT rc;

    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    UINT createDeviceFlags = 0;

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;                           // description of swapchain
    ZeroMemory(&sd, sizeof(sd));                       // clearing because some bytes
    sd.BufferCount = 1;                                // one buffer because one swapchain
    sd.BufferDesc.Width = width;                       // width
    sd.BufferDesc.Height = height;                     // height
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // pixel format
    sd.BufferDesc.RefreshRate.Numerator = 240;         // refreshing rate
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    auto& backend_data = const_cast<SDK::BackendData_t&>(g_Renderer->getBackendData());
    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels,
                                           numFeatureLevels, D3D11_SDK_VERSION, &sd, &backend_data.swap_chain,
                                           &backend_data.device, &g_featureLevel, &backend_data.device_ctx);

        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // RenderTargetOutput - forward buffer
    // RenderTargetView - Back buffer

    ID3D11Texture2D* pBackBuffer = NULL;
    hr = backend_data.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
        return hr;

    hr = backend_data.device->CreateRenderTargetView(pBackBuffer, NULL, &backend_data.render_target_view);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    backend_data.device_ctx->OMSetRenderTargets(1, &backend_data.render_target_view, NULL);
    // Viewport

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;

    // Attaching viewports to Context
    backend_data.device_ctx->RSSetViewports(1, &vp);
    return S_OK;
}

void CleanupDevice()
{
    // Clearing Context's state
    g_Renderer->Cleanup();
}


SDK::RenderContext context("global");
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInst = hInstance;

    HRESULT hr = InitWindow(nCmdShow);
    if (FAILED(hr))
    {
        LOGE("Failed InitWindow() 0x%x", hr);
        return 1;
    }
    else
    {
        LOGI("InitWindow()");
    }

    hr = InitDevice();
    if (FAILED(hr))
    {
        LOGE("Failed InitDevice() 0x%x", hr);
        CleanupDevice();
        return 1;
    }
    else
    {
        LOGI("InitDevice()");
    }

    if (g_Renderer->Initialize())
    {
        LOGE("Failed g_Renderer->Initialize()");
        CleanupDevice();
        return 0;
    }
    else
    {
        LOGI("g_Renderer->Initialize()");
    }



    g_Renderer->AddRenderContext(&context);


    // for (size_t i = 20000; i--;)
    // {
    //     context.AddRect({{0.0,0.0}, {0+i/2000.f, 0+i/2000.f}, 1111});
    // }

    // SDK::onKeyClicked.add([](VKey key){
    //     LOGI("clicked %i", key)
    // });

    // SDK::onKeyPressed.add([](VKey key){
    //     LOGI("Pressed %i", key)
    // });

    // SDK::onKeyReleased.add([](VKey key){
    //     LOGI("released %i", key)
    // });

    std::vector<uint8_t> test_image_bytes = {
        #embed "res/test_image.jpg"
    };

    std::vector<uint8_t> test_image2_bytes = {
        #embed "res/rape.jpg"
    };


    std::vector<uint8_t> arial_bytes = {
        #embed "res/ARIAL.TTF"
    };

    auto font_instance = new SDK::Font("test_font");

    font_instance->Load(arial_bytes);



    // auto image_loader = new SDK::Texture("test_image");
    // auto image_loader2 = new SDK::Texture("test_image2");

    // bool image_loaded = image_loader->Load(test_image_bytes);
    // bool image_loaded2 = image_loader2->Load(test_image2_bytes);


    MSG msg = {};

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // context.DrawRect({.min = {0,0}, .max = {0.25,0.25}, .color = 0xFFBBBBBB });
        // for (size_t i = 100000; i--;) context.DrawRect({.min = {0.4, 0.0}, .max = {0.5, 0.1}, .color = 0xFFFFFFFF});


        // context.DrawRect({.min = {0.5,0}, .max = {0.75,0.25}, .color = 0xFFFFFFFF });
        g_Renderer->Render();
    }

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
        // context.DrawRect({{0.0,0.0}, {0.5,0.5}, 0xAAFFFFFF});
        // g_Renderer->Render();
        break;
    case WM_SIZE:
        gWidth = (UINT)LOWORD(lParam);
        gHeight = (UINT)HIWORD(lParam);
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
    return 0;
}
