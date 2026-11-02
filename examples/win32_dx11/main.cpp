/*
 * Standalone Rendering example: window creation is app-side,
 * D3D11 resources are created by Iwa::Renderer::CreateDevice.
 */

#include "iwa/directx11.hpp"
#include "iwa/windows.hpp"
#include "utils/logger.hpp"
#include "utils/math.hpp"

#include <unordered_set>
#include <windows.h>

HINSTANCE HInstance = NULL;
HWND Hwnd = NULL;
auto MainRenderer = Iwa::Renderer::New("main");

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void OnRenderMain()
{
    MainRenderer->PushScissorRect({200, 200, 300, 300});
    MainRenderer->RenderFilledRect({200, 200, 400, 800}, {255, 255, 255, 255});
    MainRenderer->PopScissorRect();
    MainRenderer->PushScissorRect({200, 200, 500, 500});
    MainRenderer->RenderFilledRect({200, 200, 400, 800}, {255, 255, 255, 128});
    MainRenderer->PopScissorRect();
    MainRenderer->RenderFilledRect({200, 200, 400, 800}, {255, 255, 255, 64});

    MainRenderer->RenderBezierQuadratic({100, 100}, {400, 50}, {700, 300}, {255, 80, 80, 255}, 3.0f, 48);
    MainRenderer->RenderBezierCubic({100, 400}, {250, 150}, {550, 650}, {700, 400}, {80, 180, 255, 255}, 3.0f, 64);
}

void RenderFrame()
{
    MainRenderer->PreRender();
    MainRenderer->OnRender();
    MainRenderer->Render();
    Iwa::LogDebug("{}", MainRenderer->getDebugString());
}

HRESULT InitWindow(int nCmdShow)
{
    SetProcessDPIAware();

    const wchar_t windowName[] = L"Window Class";

    WNDCLASSW wnd{};
    wnd.style = CS_HREDRAW | CS_VREDRAW;
    wnd.hCursor = LoadCursor(0, IDC_ARROW);
    wnd.hIcon = LoadIcon(0, IDI_WINLOGO);
    wnd.lpfnWndProc = WndProc;
    wnd.hInstance = HInstance;
    wnd.lpszClassName = windowName;

    RegisterClassW(&wnd);

    Hwnd = CreateWindowExW(0, windowName, L"iwagui", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                           CW_USEDEFAULT, NULL, NULL, HInstance, NULL);

    if (!Hwnd)
        return E_FAIL;

    ShowWindow(Hwnd, nCmdShow);
    return S_OK;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstPrev;
    (void)lpCmdLine;
    HInstance = hInstance;

    HRESULT hr = InitWindow(nCmdShow);
    if (FAILED(hr))
    {
        Iwa::LogFatal("Failed InitWindow() {:#x}", hr);
        return 1;
    }
    Iwa::LogInfo("InitWindow()");

    // Required before CreateDevice/Initialize when IWA_SR_AUTO_RESIZE is enabled.
    MainRenderer->onRenderSignal.add(OnRenderMain);

    if (!MainRenderer->CreateDevice(Hwnd))
    {
        Iwa::LogFatal("Failed MainRenderer->CreateDevice()");
        MainRenderer->Destroy();
        return 1;
    }
    Iwa::LogInfo("MainRenderer->CreateDevice()");

    MSG msg = {};
    std::unordered_set<UINT> exit_codes = {WM_QUIT, WM_CLOSE, WM_DESTROY, WM_NCDESTROY};

    while (!exit_codes.contains(msg.message))
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            RenderFrame();
        }
    }

    MainRenderer->Destroy();
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (Iwa::WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    switch (uMsg)
    {
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
