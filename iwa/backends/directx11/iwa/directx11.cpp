#include <windef.h>
#include <windows.h>
#include <wingdi.h>

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#include "iwa/directx11.hpp"
#include "iwa/windows.hpp"
#include "abstractions/abstractions.hpp"
#include "utils/math.hpp"
#include "abstractions/rendering/renderer.hpp"
#include "utils/logger.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <atomic>
#include <iterator>
#include <thread>
#include <utility>
#include <vector>

#include <assert.h>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#include <inplace_vector>
#include <minwindef.h>
#include <ratio>
#include <vector>
#include <winerror.h>
#include <winnt.h>


using namespace Iwa;
using namespace Iwa::__private__;

namespace
{
    constexpr Rect_t WinToIwaRect(RECT rect)
    {
        return {
            static_cast<float>(rect.left),
            static_cast<float>(rect.top),
            static_cast<float>(rect.right),
            static_cast<float>(rect.bottom)
        };
    }

    HRESULT CompileShader(const uint8_t* shader_data, size_t shader_size, LPCSTR szEntryPoint, LPCSTR szShaderModel,
                          ID3DBlob** ppBlobOut)
    {
        HRESULT result = S_OK;
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
        ID3DBlob* pErrorBlob;

        result = D3DCompile(shader_data, shader_size, nullptr, nullptr, nullptr, szEntryPoint, szShaderModel,
                            dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

        if (FAILED(result))
        {
            if (pErrorBlob != nullptr)
                OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

            if (pErrorBlob)
                pErrorBlob->Release();
            return result;
        }
        if (pErrorBlob)
            pErrorBlob->Release();
        return S_OK;
    }
} // namespace

Texture::Texture(const Renderer* renderer, const std::string& id)
    : AbstractTexture<Texture, Renderer, ID3D11ShaderResourceView>::AbstractTexture(renderer, id)
{
}

Texture::~Texture()
{
    if (getResource())
        getResource()->Release();
}

bool Texture::Load(const std::span<const uint8_t> data)
{
    HRESULT result = S_OK;
    auto& bd = renderer->backend_data;

    int image_width, image_height = 0;
    uint8_t* image_data = stbi_load_from_memory(data.data(), data.size(), &image_width, &image_height, nullptr, 4);

    __size = Vec2_t(image_width, image_height);
    if (image_data == nullptr)
    {
        LogFatal("Couldn't load image_data: stbi_load_from_memory returned nullptr.");
    }

    ID3D11Texture2D* texture = nullptr;
    D3D11_TEXTURE2D_DESC tex_desc = {};
    tex_desc.Width = static_cast<int>(__size.x);
    tex_desc.Height = static_cast<int>(__size.y);
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.Usage = D3D11_USAGE_DEFAULT;
    tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    tex_desc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA subres = {};
    subres.pSysMem = image_data;
    subres.SysMemPitch = tex_desc.Width * 4;
    subres.SysMemSlicePitch = 0;
    result = bd.device->CreateTexture2D(&tex_desc, &subres, &texture);
    if (FAILED(result))
    {
        LogFatal("Couldn't create Texture2D, code: {:#x}", result);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = tex_desc.MipLevels;
    srv_desc.Texture2D.MostDetailedMip = 0;

    result = bd.device->CreateShaderResourceView(texture, &srv_desc, &__resource);
    texture->Release();

    if (FAILED(result))
    {
        LogFatal("Couldn't create shader resource view, code: {:#x}", result);
    }

    stbi_image_free(image_data);

    return SUCCEEDED(result);
}

bool Texture::Load(const std::filesystem::path& path)
{
    IWA_ASSERT(not std::filesystem::is_regular_file(path) &&
               std::format("File {} is not existing.", path.string()).data());

    std::ifstream file_input(path);
    std::vector<uint8_t> file_data((std::istreambuf_iterator<char>(file_input)), std::istreambuf_iterator<char>());

    return Load(file_data);
}

BitmapFont::BitmapFont(const Renderer* renderer, const std::string& id, float scale)
    : AbstractBitmapFont<BitmapFont, Renderer, ID3D11ShaderResourceView>(renderer, id, scale)
{
}

BitmapFont::~BitmapFont()
{
    if (isLoaded())
        getResource()->Release();
}

BitmapFont* BitmapFont::Load(const std::span<const uint8_t> data, const std::span<const CharRange_t> ranges)
{
    auto pixels = __alloc_and_pack_font_bitmap(data, ranges);

    HRESULT result = S_OK;
    auto& bd = renderer->backend_data;

    ID3D11Texture2D* texture = nullptr;
    D3D11_TEXTURE2D_DESC tex_desc = {};
    tex_desc.Width = __atlas_size.x;
    tex_desc.Height = __atlas_size.y;
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Format = DXGI_FORMAT_R8_UNORM;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
    tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subres = {};
    subres.pSysMem = pixels->data();
    subres.SysMemPitch = __atlas_size.x;

    result = bd.device->CreateTexture2D(&tex_desc, &subres, &texture);

    if (FAILED(result))
    {
        LogFatal("Coulnd't create glyph texture, code: {:#x}", result);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = DXGI_FORMAT_R8_UNORM;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;

    result = bd.device->CreateShaderResourceView(texture, &srv_desc, &__resource);
    texture->Release();
    delete pixels;

    if (FAILED(result))
    {
        LogFatal("Coulnd't create glyph shader resource, code: {:#x}", result);
    }

    return this;
}

BitmapFont* BitmapFont::Load(const std::filesystem::path& path, const std::span<const CharRange_t> ranges)
{
    return this;
}

BitmapFont* BitmapFont::LoadFromSystem(const std::filesystem::path& path,
                                       const std::span<const CharRange_t> ranges)
{
    return Load(std::filesystem::path("C:/Windows/Fonts") / path, ranges);
}

DynamicFont::DynamicFont(const Renderer* renderer, const std::string& id, float bake_size)
    : AbstractDynamicFont<DynamicFont, Renderer, ID3D11ShaderResourceView>(renderer, id, bake_size)
{
}

DynamicFont::~DynamicFont()
{
    if (isLoaded())
        getResource()->Release();
}

DynamicFont* DynamicFont::Load(const std::span<const uint8_t> data, const std::span<const CharRange_t> ranges)
{
    auto pixels = __alloc_and_pack_font_sdf(data, ranges);

    HRESULT result = S_OK;
    auto& bd = renderer->backend_data;

    ID3D11Texture2D* texture = nullptr;
    D3D11_TEXTURE2D_DESC tex_desc = {};
    tex_desc.Width = static_cast<UINT>(__atlas_size.x);
    tex_desc.Height = static_cast<UINT>(__atlas_size.y);
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Format = DXGI_FORMAT_R8_UNORM;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
    tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subres = {};
    subres.pSysMem = pixels->data();
    subres.SysMemPitch = static_cast<UINT>(__atlas_size.x);

    result = bd.device->CreateTexture2D(&tex_desc, &subres, &texture);
    if (FAILED(result))
    {
        LogFatal("Couldn't create SDF glyph texture, code: {:#x}", result);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = DXGI_FORMAT_R8_UNORM;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;

    result = bd.device->CreateShaderResourceView(texture, &srv_desc, &__resource);
    texture->Release();
    delete pixels;

    if (FAILED(result))
    {
        LogFatal("Couldn't create SDF glyph shader resource, code: {:#x}", result);
    }

    return this;
}

DynamicFont* DynamicFont::Load(const std::filesystem::path& path, const std::span<const CharRange_t> ranges)
{
    IWA_ASSERT(std::filesystem::is_regular_file(path) &&
               std::format("File {} is not existing.", path.string()).data());

    std::ifstream file_input(path, std::ios::binary);
    std::vector<uint8_t> file_data((std::istreambuf_iterator<char>(file_input)), std::istreambuf_iterator<char>());
    return Load(file_data, ranges);
}

DynamicFont* DynamicFont::LoadFromSystem(const std::filesystem::path& path,
                                         const std::span<const CharRange_t> ranges)
{
    return Load(std::filesystem::path("C:/Windows/Fonts") / path, ranges);
}

Renderer::Renderer(const std::string& id) : AbstractRenderer<Renderer, __private__::BackendData_t, Texture, ID3D11ShaderResourceView, BitmapFont, DynamicFont>(id) {}

#ifdef IWA_SR_AUTO_RESIZE
void Renderer::__install_sr_auto_resize_hook()
{
    this->__ensure_on_render_hooks();

    if (__sr_wndproc_hook_installed)
        return;

    __sr_wndproc_hook = [this](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg != WM_SIZE || hwnd != backend_data.hwnd || wParam == SIZE_MINIMIZED)
            return;
        if (backend_data.device == nullptr || backend_data.swap_chain == nullptr)
            return;
        if (__sr_handling_resize)
            return;

        __sr_handling_resize = true;
        this->ResizeViewport();
        this->__ensure_on_render_hooks();
        this->PreRender();
        this->OnRender();
        this->Render();
        __sr_handling_resize = false;
    };

    __sr_wndproc_hook_id = onWndProcSignal.connect(__sr_wndproc_hook);
    __sr_wndproc_hook_installed = true;
    LogDebug("Installed WndProc WM_SIZE hook");
}

void Renderer::__uninstall_sr_auto_resize_hook()
{
    if (!__sr_wndproc_hook_installed)
        return;

    onWndProcSignal.disconnect(__sr_wndproc_hook_id);
    __sr_wndproc_hook_id = 0;
    __sr_wndproc_hook = {};
    __sr_wndproc_hook_installed = false;
    __sr_handling_resize = false;
}
#endif

void Renderer::__release_device_resources()
{
#ifdef IWA_SR_AUTO_RESIZE
    __uninstall_sr_auto_resize_hook();
#endif

    const auto release = [](auto*& ptr) {
        if (ptr)
        {
            ptr->Release();
            ptr = nullptr;
        }
    };

    release(backend_data.index_buffer.ptr);
    backend_data.index_buffer.size = 0;
    release(backend_data.vertex_buffer.ptr);
    backend_data.vertex_buffer.size = 0;
    release(backend_data.constant_buffer);
    release(backend_data.sampler_state);
    release(backend_data.sampler_state_linear);
    release(backend_data.blend_state);
    release(backend_data.rasterizer_state);
    release(backend_data.disabled_stencil_state);

    if (backend_data.device_ctx)
        backend_data.device_ctx->ClearState();

    release(backend_data.vertex_layout);
    release(backend_data.vertex_shader);
    release(backend_data.pixel_shader);
    release(backend_data.pixel_shader_textured);
    release(backend_data.pixel_shader_text);
    release(backend_data.pixel_shader_text_sdf);
    release(backend_data.pixel_shader_shadow);
    release(backend_data.pixel_shader_inner_shadow);
    release(backend_data.pixel_shader_outline);
    release(backend_data.pixel_shader_circle);
    release(backend_data.pixel_shader_circle_textured);
    release(backend_data.pixel_shader_circle_shadow);
    release(backend_data.pixel_shader_circle_inner_shadow);
    release(backend_data.pixel_shader_line_shadow);
    release(backend_data.render_target_view);
    release(backend_data.swap_chain);
    release(backend_data.device_ctx);
    release(backend_data.device);
    backend_data.hwnd = nullptr;
}

Renderer::~Renderer()
{
    Iwa::BitmapFont::Clear();
    Iwa::DynamicFont::Clear();
    __release_device_resources();
}

HRESULT Renderer::Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* immediate_context,
                             IDXGISwapChain* swap_chain, ID3D11RenderTargetView* render_target_view)
{
    backend_data.hwnd = hwnd;
    backend_data.device = device;
    backend_data.device_ctx = immediate_context;
    backend_data.swap_chain = swap_chain;
    backend_data.render_target_view = render_target_view;

    return Initialize();
}

#ifdef IWA_SR
bool Renderer::CreateDevice(HWND hwnd)
{
    if (hwnd == nullptr)
    {
        LogFatal("CreateDevice: hwnd is null");
        return false;
    }

    DestroyDevice();

    LogDebug("Standalone CreateDevice for renderer \"{}\"", getID());
    LogPushTab();

    RECT rc{};
    GetClientRect(hwnd, &rc);
    const UINT width = static_cast<UINT>(std::max(1L, rc.right - rc.left));
    const UINT height = static_cast<UINT>(std::max(1L, rc.bottom - rc.top));

    const D3D_DRIVER_TYPE driver_types[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    const D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    backend_data.hwnd = hwnd;

    HRESULT result = E_FAIL;
    D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
    for (const D3D_DRIVER_TYPE driver_type : driver_types)
    {
        result = D3D11CreateDevice(nullptr, driver_type, nullptr, 0, feature_levels, ARRAYSIZE(feature_levels),
                                   D3D11_SDK_VERSION, &backend_data.device, &feature_level, &backend_data.device_ctx);
        if (SUCCEEDED(result))
        {
            LogDebug("Created device, driver type: {}, feature level: {:#x}", static_cast<uint64_t>(driver_type),
                     static_cast<uint32_t>(feature_level));
            break;
        }
    }

    if (FAILED(result))
    {
        LogFatal("Couldn't create device, code: {:#x}", result);
        LogPopTab();
        DestroyDevice();
        return false;
    }

    IDXGIFactory2* dxgi_factory = nullptr;
    {
        IDXGIDevice* dxgi_device = nullptr;
        result = backend_data.device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device));
        if (SUCCEEDED(result))
        {
            IDXGIAdapter* dxgi_adapter = nullptr;
            result = dxgi_device->GetAdapter(&dxgi_adapter);
            if (SUCCEEDED(result))
            {
                result = dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgi_factory));
                dxgi_adapter->Release();
            }
            dxgi_device->Release();
        }
    }

    if (FAILED(result) || dxgi_factory == nullptr)
    {
        LogFatal("Couldn't get DXGI factory, code: {:#x}", result);
        LogPopTab();
        DestroyDevice();
        return false;
    }

    // DXGI_SCALING_NONE: do not stretch buffers when window size temporarily mismatches.
    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{};
    swap_chain_desc.Width = width;
    swap_chain_desc.Height = height;
    swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.Scaling = DXGI_SCALING_NONE;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    IDXGISwapChain1* swap_chain1 = nullptr;
    result = dxgi_factory->CreateSwapChainForHwnd(backend_data.device, hwnd, &swap_chain_desc, nullptr, nullptr,
                                                  &swap_chain1);
    dxgi_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    dxgi_factory->Release();

    if (FAILED(result))
    {
        LogFatal("Couldn't create swap chain, code: {:#x}", result);
        LogPopTab();
        DestroyDevice();
        return false;
    }

    result = swap_chain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&backend_data.swap_chain));
    swap_chain1->Release();
    if (FAILED(result))
    {
        LogFatal("Couldn't query IDXGISwapChain, code: {:#x}", result);
        LogPopTab();
        DestroyDevice();
        return false;
    }

    ID3D11Texture2D* back_buffer = nullptr;
    result = backend_data.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));
    if (FAILED(result))
    {
        LogFatal("Couldn't get back buffer, code: {:#x}", result);
        LogPopTab();
        DestroyDevice();
        return false;
    }

    result = backend_data.device->CreateRenderTargetView(back_buffer, nullptr, &backend_data.render_target_view);
    back_buffer->Release();
    if (FAILED(result))
    {
        LogFatal("Couldn't create render target view, code: {:#x}", result);
        LogPopTab();
        DestroyDevice();
        return false;
    }

    backend_data.device_ctx->OMSetRenderTargets(1, &backend_data.render_target_view, nullptr);

    backend_data.viewport.Width = static_cast<FLOAT>(width);
    backend_data.viewport.Height = static_cast<FLOAT>(height);
    backend_data.viewport.MinDepth = 0.0f;
    backend_data.viewport.MaxDepth = 1.0f;
    backend_data.viewport.TopLeftX = 0.0f;
    backend_data.viewport.TopLeftY = 0.0f;
    backend_data.device_ctx->RSSetViewports(1, &backend_data.viewport);

    result = Initialize();
    if (FAILED(result))
    {
        LogFatal("Standalone CreateDevice: Initialize failed, code: {:#x}", result);
        LogPopTab();
        DestroyDevice();
        return false;
    }

    LogPopTab();
    LogDebug("Standalone CreateDevice success");
    return true;
}

void Renderer::DestroyDevice()
{
    __release_device_resources();
}
#endif

HRESULT Renderer::Initialize()
{
    LogDebug("Initializing renderer \"{}\" with render id \"{}\"", getID(), getRendererID());
    LogPushTab();

    IWA_ASSERT(backend_data.device != nullptr && "\"__bd.device\" is null");
    IWA_ASSERT(backend_data.device_ctx != nullptr && "\"__bd.immediate_context\" is null");
    IWA_ASSERT(backend_data.swap_chain != nullptr && "\"__bd.swap_chain\" is null");
    IWA_ASSERT(backend_data.render_target_view != nullptr && "\"__bd.render_target_view\" is null");
    IWA_ASSERT(backend_data.hwnd != nullptr && "\"__bd.hwnd\" is null");

    HRESULT result = S_OK;

    constexpr uint8_t shader_data[] = {
#embed "shaders.hlsl"
    };

    struct PixelShaderJob
    {
        const char* entry;
        ID3D11PixelShader** out;
    };

    const PixelShaderJob ps_jobs[] = {
        {"PS", &backend_data.pixel_shader},
        {"PSTextured", &backend_data.pixel_shader_textured},
        {"PSText", &backend_data.pixel_shader_text},
        {"PSTextSDF", &backend_data.pixel_shader_text_sdf},
        {"PSShadow", &backend_data.pixel_shader_shadow},
        {"PSInnerShadow", &backend_data.pixel_shader_inner_shadow},
        {"PSOutline", &backend_data.pixel_shader_outline},
        {"PSCircle", &backend_data.pixel_shader_circle},
        {"PSCircleTextured", &backend_data.pixel_shader_circle_textured},
        {"PSCircleShadow", &backend_data.pixel_shader_circle_shadow},
        {"PSCircleInnerShadow", &backend_data.pixel_shader_circle_inner_shadow},
        {"PSLineShadow", &backend_data.pixel_shader_line_shadow},
    };

    struct CompileJob
    {
        const char* entry;
        const char* model;
    };

    struct CompileResult
    {
        HRESULT hr = E_FAIL;
        ID3DBlob* blob = nullptr;
    };

    std::vector<CompileJob> compile_jobs;
    compile_jobs.reserve(1 + std::size(ps_jobs));
    compile_jobs.push_back({"VS", "vs_4_0"});
    for (const auto& job : ps_jobs)
        compile_jobs.push_back({job.entry, "ps_4_0"});

    std::vector<CompileResult> compile_results(compile_jobs.size());
    std::atomic<size_t> next_job{0};
    const unsigned worker_count = std::max(
        1u, std::min(static_cast<unsigned>(compile_jobs.size()), std::max(1u, std::thread::hardware_concurrency())));

    auto worker_fn = [&]() {
        for (;;)
        {
            const size_t job_index = next_job.fetch_add(1, std::memory_order_relaxed);
            if (job_index >= compile_jobs.size())
                break;

            ID3DBlob* blob = nullptr;
            const HRESULT hr = CompileShader(shader_data, sizeof(shader_data), compile_jobs[job_index].entry,
                                             compile_jobs[job_index].model, &blob);
            compile_results[job_index].hr = hr;
            compile_results[job_index].blob = blob;
        }
    };

    {
        std::vector<std::thread> workers;
        workers.reserve(worker_count);
        for (unsigned i = 0; i < worker_count; ++i)
            workers.emplace_back(worker_fn);
        for (auto& worker : workers)
            worker.join();
    }

    auto release_compile_blobs_from = [&](size_t begin) {
        for (size_t i = begin; i < compile_results.size(); ++i)
        {
            if (compile_results[i].blob)
            {
                compile_results[i].blob->Release();
                compile_results[i].blob = nullptr;
            }
        }
    };

    auto& vs_result = compile_results[0];
    if (FAILED(vs_result.hr))
    {
        LogFatal("Couldn't compile vertex shader, code: {:#x}", vs_result.hr);
        release_compile_blobs_from(0);
        return vs_result.hr;
    }
    LogDebug("Vertex shader compiled");

    ID3DBlob* vs_blob = vs_result.blob;
    result = backend_data.device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr,
                                                     &backend_data.vertex_shader);
    if (FAILED(result))
    {
        LogFatal("Couldn't create vertex shader, code: {:#x}", result);
        release_compile_blobs_from(0);
        return result;
    }
    LogDebug("Vertex shader created");

    constexpr D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetof(Vertex_t, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)offsetof(Vertex_t, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetof(Vertex_t, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(Vertex_t, rect_w), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(Vertex_t, shadow_thickness),
         D3D11_INPUT_PER_VERTEX_DATA, 0}};

    result = backend_data.device->CreateInputLayout(layout, ARRAYSIZE(layout), vs_blob->GetBufferPointer(),
                                                    vs_blob->GetBufferSize(), &backend_data.vertex_layout);
    LogDebug("Input layout created");
    vs_blob->Release();
    vs_result.blob = nullptr;

    if (FAILED(result))
    {
        LogFatal("Couldn't create input layout, code: {:#x}", result);
        release_compile_blobs_from(1);
        return result;
    }
    backend_data.device_ctx->IASetInputLayout(backend_data.vertex_layout);

    LogDebug("Pixel shaders ({}):", std::size(ps_jobs));
    LogPushTab();

    for (size_t i = 0; i < std::size(ps_jobs); ++i)
    {
        auto& ps_compiled = compile_results[i + 1];
        if (FAILED(ps_compiled.hr))
        {
            LogFatal("Couldn't compile \"{}\", code: {:#x}", ps_jobs[i].entry, ps_compiled.hr);
            release_compile_blobs_from(i + 1);
            return ps_compiled.hr;
        }

        ID3DBlob* ps_blob = ps_compiled.blob;
        result = backend_data.device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr,
                                                        ps_jobs[i].out);
        ps_blob->Release();
        ps_compiled.blob = nullptr;

        if (FAILED(result))
        {
            LogFatal("Couldn't create \"{}\", code: {:#x}", ps_jobs[i].entry, result);
            release_compile_blobs_from(i + 2);
            return result;
        }

        LogDebug("\"{}\" success", ps_jobs[i].entry);
    }

    LogPopTab();
    LogDebug("Pixel shaders success");

    backend_data.device_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D11_DEPTH_STENCIL_DESC stencil_desc = {};
    stencil_desc.DepthEnable = false;
    stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
    stencil_desc.StencilEnable = false;
    stencil_desc.StencilReadMask = 0xFF;
    stencil_desc.StencilWriteMask = 0xFF;
    stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    result = backend_data.device->CreateDepthStencilState(&stencil_desc, &backend_data.disabled_stencil_state);
    if (FAILED(result))
    {
        LogFatal("Couldn't create depth stencil state, code: {:#x}", result);
        return result;
    }

    backend_data.device_ctx->OMSetDepthStencilState(backend_data.disabled_stencil_state, 1);

    // 2D UI: no back-face cull — line/outline quads flip winding with direction.
    D3D11_RASTERIZER_DESC raster_desc = {};
    raster_desc.FillMode = D3D11_FILL_SOLID;
    raster_desc.CullMode = D3D11_CULL_NONE;
    raster_desc.FrontCounterClockwise = FALSE;
    raster_desc.DepthClipEnable = TRUE;
    raster_desc.ScissorEnable = TRUE;
    raster_desc.MultisampleEnable = FALSE;
    raster_desc.AntialiasedLineEnable = FALSE;
    result = backend_data.device->CreateRasterizerState(&raster_desc, &backend_data.rasterizer_state);
    if (FAILED(result))
    {
        LogFatal("Couldn't create rasterizer state, code: {:#x}", result);
        return result;
    }
    backend_data.device_ctx->RSSetState(backend_data.rasterizer_state);

    D3D11_BLEND_DESC blend_desc = {};
    blend_desc.RenderTarget[0].BlendEnable = TRUE;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA; // D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    backend_data.device->CreateBlendState(&blend_desc, &backend_data.blend_state);

    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.MipLODBias = 0.0f;
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampler_desc.MinLOD = 0.0f;
    sampler_desc.MaxLOD = 0.0f;

    result = backend_data.device->CreateSamplerState(&sampler_desc, &backend_data.sampler_state);

    if (FAILED(result))
    {
        LogFatal("Couldn't create sampler state, code: {:#x}", result);
        return result;
    }

    D3D11_SAMPLER_DESC linear_sampler_desc = sampler_desc;
    linear_sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    result = backend_data.device->CreateSamplerState(&linear_sampler_desc, &backend_data.sampler_state_linear);
    if (FAILED(result))
    {
        LogFatal("Couldn't create linear sampler state, code: {:#x}", result);
        return result;
    }

    D3D11_BUFFER_DESC constant_buffer_desc{};
    constant_buffer_desc.ByteWidth = sizeof(ConstantBuffer_t);
    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_buffer_desc.MiscFlags = 0;

    result = backend_data.device->CreateBuffer(&constant_buffer_desc, nullptr, &backend_data.constant_buffer);

    if (FAILED(result))
    {
        LogFatal("Couldn't create constant buffer, code: {:#x}", result);
    }

    this->ResizeViewport();

#ifdef IWA_SR_AUTO_RESIZE
    __install_sr_auto_resize_hook();
#endif

    LogPopTab();

    return result;
}

void Renderer::PreRender()
{
    this->__pre_render_impl();
}

void Renderer::Render()
{
#ifdef IWA_FRAME_DEBUG_INFO
    this->__scissor_rects_count = this->__scissor_queue.size();
#endif

    __frame_count += 1;

    float ClearColor[4] = {0.35f, 0.35f, 0.5f, 1.0f};
    float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    backend_data.device_ctx->OMSetRenderTargets(1, &backend_data.render_target_view, nullptr);
    backend_data.device_ctx->RSSetViewports(1, &backend_data.viewport);
    backend_data.device_ctx->OMSetBlendState(backend_data.blend_state, blendFactor, 0xffffffff);
    backend_data.device_ctx->RSSetState(backend_data.rasterizer_state);
    backend_data.device_ctx->OMSetDepthStencilState(backend_data.disabled_stencil_state, 1);
    backend_data.device_ctx->ClearRenderTargetView(backend_data.render_target_view, ClearColor);
    backend_data.device_ctx->VSSetShader(backend_data.vertex_shader, nullptr, 0);
    backend_data.device_ctx->VSSetConstantBuffers(0, 1, &backend_data.constant_buffer);
    backend_data.device_ctx->PSSetSamplers(0, 1, &backend_data.sampler_state);

    constexpr UINT stride = static_cast<UINT>(sizeof(Vertex_t));
    constexpr UINT vertex_offset = 0;

    HRESULT result = S_OK;

    D3D11_MAPPED_SUBRESOURCE const_buf_resource;

    if (SUCCEEDED(backend_data.device_ctx->Map(backend_data.constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0,
                                               &const_buf_resource)))
    {
        auto constant_buffer = static_cast<ConstantBuffer_t*>(const_buf_resource.pData);

        auto [left, top] = this->__viewport_rect.min;
        auto [right, bottom] = this->__viewport_rect.max;

        float model_view_projection[4][4] = {
            {2.0f / (right - left), 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / (top - bottom), 0.0f, 0.0f},
            {0.0f, 0.0f, 0.5f, 0.0f},
            {(right + left) / (left - right), (top + bottom) / (bottom - top), 0.5f, 1.0f},
        };

        std::memcpy(&constant_buffer->model_view_projection, model_view_projection, sizeof(model_view_projection));
        backend_data.device_ctx->Unmap(backend_data.constant_buffer, 0);
    }

    if (!backend_data.vertex_buffer.ptr || backend_data.vertex_buffer.size < __vertices.size())
    {
        if (backend_data.vertex_buffer.ptr)
            backend_data.vertex_buffer.ptr->Release();

        backend_data.vertex_buffer.size = __vertices.size() * sizeof(Vertex_t) + 4096;
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = __vertices.size() * sizeof(Vertex_t) + 4096;
        desc.MiscFlags = 0;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        result = backend_data.device->CreateBuffer(&desc, nullptr, &backend_data.vertex_buffer.ptr);
        if (FAILED(result))
            LogFatal("Failed resizing vertex buffer, code: {:#x}", result);
    }

    if (!backend_data.index_buffer.ptr || backend_data.index_buffer.size < __indices.size())
    {
        if (backend_data.index_buffer.ptr)
            backend_data.index_buffer.ptr->Release();

        backend_data.index_buffer.size = __indices.size() * sizeof(IndexSize_t) + 4096;
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = backend_data.index_buffer.size;
        desc.MiscFlags = 0;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT result = backend_data.device->CreateBuffer(&desc, nullptr, &backend_data.index_buffer.ptr);
        if (FAILED(result))
            LogFatal("Failed resizing index buffer, code: {:#x}", result);
    }

    D3D11_MAPPED_SUBRESOURCE vertex_resource, index_resource;

    result = backend_data.device_ctx->Map(backend_data.vertex_buffer.ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertex_resource);

    if (FAILED(result))
        LogFatal("Failed mapping vertex buffer, code: {:#x}", result);

    result = backend_data.device_ctx->Map(backend_data.index_buffer.ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &index_resource);

    if (FAILED(result))
        LogFatal("Failed mapping vertex buffer, code: {:#x}", result);

    auto vertex_dest = static_cast<Vertex_t*>(vertex_resource.pData);
    auto index_dest = static_cast<IndexSize_t*>(index_resource.pData);

    std::memcpy(vertex_dest, __vertices.data(), __vertices.size() * sizeof(Vertex_t));
    std::memcpy(index_dest, __indices.data(), __indices.size() * sizeof(IndexSize_t));

    backend_data.device_ctx->Unmap(backend_data.vertex_buffer.ptr, 0);
    backend_data.device_ctx->Unmap(backend_data.index_buffer.ptr, 0);

    backend_data.device_ctx->IASetVertexBuffers(0, 1, &backend_data.vertex_buffer.ptr, &stride, &vertex_offset);
    backend_data.device_ctx->IASetIndexBuffer(backend_data.index_buffer.ptr, DXGI_FORMAT_R32_UINT, 0);

    IndexSize_t index_offset = 0;
    IndexChunkType cur_chtype = IndexChunkType::RECT | IndexChunkType::FILLED;
    backend_data.device_ctx->PSSetShader(backend_data.pixel_shader, nullptr, 0);

    const auto apply_scissor = [&](const Rect_t& rect) {
        D3D11_RECT r{};
        r.left = static_cast<LONG>(std::floor(std::min(rect.min.x, rect.max.x)));
        r.top = static_cast<LONG>(std::floor(std::min(rect.min.y, rect.max.y)));
        r.right = static_cast<LONG>(std::ceil(std::max(rect.min.x, rect.max.x)));
        r.bottom = static_cast<LONG>(std::ceil(std::max(rect.min.y, rect.max.y)));
        if (r.right < r.left)
            r.right = r.left;
        if (r.bottom < r.top)
            r.bottom = r.top;
        backend_data.device_ctx->RSSetScissorRects(1, &r);
    };

    apply_scissor(this->__viewport_rect);

    for (size_t chunk_i = 0; chunk_i < __indices_chunks.size(); ++chunk_i)
    {
        auto& chunk = __indices_chunks[chunk_i];

        if (!__scissor_queue.empty())
        {
            const auto& range = __scissor_queue.front().first;
            if (chunk_i >= range.first && chunk_i < range.second)
            {
                apply_scissor(__scissor_queue.front().second);
                __scissor_queue.pop_front();
            }
            else
            {
                apply_scissor(this->__viewport_rect);
            }
        }
        else
        {
            apply_scissor(this->__viewport_rect);
        }

        if (chunk.shader_type != cur_chtype)
        {
            const IndexChunkType type = chunk.shader_type;

            if (type == IndexChunkType::TEXTURED)
            {
                backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_textured, nullptr, 0);
            }
            else if (type == IndexChunkType::TEXT)
            {
                backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_text, nullptr, 0);
                backend_data.device_ctx->PSSetSamplers(0, 1, &backend_data.sampler_state);
            }
            else if (type == IndexChunkType::TEXT_SDF)
            {
                backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_text_sdf, nullptr, 0);
                backend_data.device_ctx->PSSetSamplers(0, 1, &backend_data.sampler_state_linear);
            }
            else if (has_flag(type, IndexChunkType::LINE))
            {
                if (has_flag(type, IndexChunkType::SHADOW))
                    backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_line_shadow, nullptr, 0);
                else
                    backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_circle, nullptr, 0);
            }
            else if (has_flag(type, IndexChunkType::CIRCLE))
            {
                if (has_flag(type, IndexChunkType::TEXTURED))
                {
                    backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_circle_textured, nullptr, 0);
                    backend_data.device_ctx->PSSetSamplers(0, 1, &backend_data.sampler_state_linear);
                }
                else if (has_flag(type, IndexChunkType::SHADOW))
                    backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_circle_shadow, nullptr, 0);
                else if (has_flag(type, IndexChunkType::INNER_SHADOW))
                    backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_circle_inner_shadow, nullptr, 0);
                else
                    backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_circle, nullptr, 0);
            }
            else if (has_flag(type, IndexChunkType::SHADOW))
            {
                backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_shadow, nullptr, 0);
            }
            else if (has_flag(type, IndexChunkType::INNER_SHADOW))
            {
                backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_inner_shadow, nullptr, 0);
            }
            else if (has_flag(type, IndexChunkType::FILLED))
            {
                backend_data.device_ctx->PSSetShader(backend_data.pixel_shader, nullptr, 0);
            }
            else if (has_flag(type, IndexChunkType::RECT))
            {
                backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_outline, nullptr, 0);
            }
            else
            {
                backend_data.device_ctx->PSSetShader(backend_data.pixel_shader, nullptr, 0);
            }

            cur_chtype = chunk.shader_type;
        }

        if (chunk.shader_type == IndexChunkType::TEXTURED || chunk.shader_type == IndexChunkType::TEXT ||
            chunk.shader_type == IndexChunkType::TEXT_SDF ||
            has_flag(chunk.shader_type, IndexChunkType::CIRCLE | IndexChunkType::TEXTURED))
        {
            ID3D11ShaderResourceView* srv = static_cast<ID3D11ShaderResourceView*>(chunk.data);
            backend_data.device_ctx->PSSetShaderResources(0, 1, &srv);
        }

        backend_data.device_ctx->DrawIndexed(chunk.index - index_offset, index_offset, 0);
        index_offset = chunk.index;
    }

#ifdef IWA_SR
    #ifdef IWA_SR_LAZY_SWAP_FACTOR

    if (IsIconic(this->backend_data.hwnd)) backend_data.swap_chain->Present(static_cast<int>(2 * IWA_SR_LAZY_SWAP_FACTOR), 0);
    else if (auto f_hwnd = GetForegroundWindow() != this->backend_data.hwnd) backend_data.swap_chain->Present(static_cast<int>(1 * IWA_SR_LAZY_SWAP_FACTOR), 0);
    else backend_data.swap_chain->Present(this->isVSyncEnabled() ? 1u : 0u, 0);

    #else

    backend_data.swap_chain->Present(this->isVSyncEnabled() ? 1u : 0u, 0);

    #endif
#endif

    this->__post_render_impl();
}

void Renderer::ResizeViewport()
{
    if (backend_data.device == nullptr || backend_data.swap_chain == nullptr || backend_data.hwnd == nullptr)
        return;

    RECT rect{};
    GetClientRect(backend_data.hwnd, &rect);

    const UINT width = static_cast<UINT>(std::max(0L, rect.right - rect.left));
    const UINT height = static_cast<UINT>(std::max(0L, rect.bottom - rect.top));
    if (width == 0 || height == 0)
        return;

    if (backend_data.viewport.Width == static_cast<float>(width) &&
        backend_data.viewport.Height == static_cast<float>(height) && backend_data.render_target_view != nullptr)
    {
        this->__resize_viewport_impl(
            Rect_t{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)});
        return;
    }

    backend_data.device_ctx->OMSetRenderTargets(0, nullptr, nullptr);
    if (backend_data.render_target_view)
    {
        backend_data.render_target_view->Release();
        backend_data.render_target_view = nullptr;
    }

    HRESULT hr = backend_data.swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr))
    {
        LogError("ResizeBuffers failed, code: {:#x}", hr);
        return;
    }

    ID3D11Texture2D* back_buffer = nullptr;
    hr = backend_data.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));
    if (FAILED(hr))
    {
        LogError("GetBuffer after resize failed, code: {:#x}", hr);
        return;
    }

    D3D11_TEXTURE2D_DESC back_desc{};
    back_buffer->GetDesc(&back_desc);

    hr = backend_data.device->CreateRenderTargetView(back_buffer, nullptr, &backend_data.render_target_view);
    back_buffer->Release();
    if (FAILED(hr))
    {
        LogError("CreateRenderTargetView after resize failed, code: {:#x}", hr);
        return;
    }

    const float view_w = static_cast<float>(back_desc.Width);
    const float view_h = static_cast<float>(back_desc.Height);
    this->__resize_viewport_impl(Rect_t{0.0f, 0.0f, view_w, view_h});

    backend_data.viewport.Width = view_w;
    backend_data.viewport.Height = view_h;
    backend_data.viewport.MinDepth = 0.0f;
    backend_data.viewport.MaxDepth = 1.0f;
    backend_data.viewport.TopLeftX = 0.0f;
    backend_data.viewport.TopLeftY = 0.0f;
    backend_data.device_ctx->RSSetViewports(1, &backend_data.viewport);
    backend_data.device_ctx->OMSetRenderTargets(1, &backend_data.render_target_view, nullptr);
}
