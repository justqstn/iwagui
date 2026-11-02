#include <unordered_map>
#include <windef.h>
#include <windows.h>
#include <wingdi.h>

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#include "iwa/directx11.hpp"
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
#include <iterator>

#include <assert.h>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <d3d11.h>
#include <d3dcompiler.h>
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

Renderer::~Renderer()
{
    if (backend_data.index_buffer.ptr)
        backend_data.index_buffer.ptr->Release();
    if (backend_data.vertex_buffer.ptr)
        backend_data.vertex_buffer.ptr->Release();
    if (backend_data.constant_buffer)
        backend_data.constant_buffer->Release();
    if (backend_data.sampler_state)
        backend_data.sampler_state->Release();
    if (backend_data.sampler_state_linear)
        backend_data.sampler_state_linear->Release();
    if (backend_data.blend_state)
        backend_data.blend_state->Release();
    if (backend_data.rasterizer_state)
        backend_data.rasterizer_state->Release();
    if (backend_data.disabled_stencil_state)
        backend_data.disabled_stencil_state->Release();
    if (backend_data.device_ctx)
        backend_data.device_ctx->ClearState();
    if (backend_data.vertex_layout)
        backend_data.vertex_layout->Release();
    if (backend_data.vertex_shader)
        backend_data.vertex_shader->Release();
    if (backend_data.pixel_shader)
        backend_data.pixel_shader->Release();
    if (backend_data.pixel_shader_textured)
        backend_data.pixel_shader_textured->Release();
    if (backend_data.pixel_shader_text)
        backend_data.pixel_shader_text->Release();
    if (backend_data.pixel_shader_text_sdf)
        backend_data.pixel_shader_text_sdf->Release();
    if (backend_data.pixel_shader_shadow)
        backend_data.pixel_shader_shadow->Release();
    if (backend_data.pixel_shader_inner_shadow)
        backend_data.pixel_shader_inner_shadow->Release();
    if (backend_data.pixel_shader_outline)
        backend_data.pixel_shader_outline->Release();
    if (backend_data.pixel_shader_circle)
        backend_data.pixel_shader_circle->Release();
    if (backend_data.pixel_shader_circle_textured)
        backend_data.pixel_shader_circle_textured->Release();
    if (backend_data.pixel_shader_circle_shadow)
        backend_data.pixel_shader_circle_shadow->Release();
    if (backend_data.pixel_shader_circle_inner_shadow)
        backend_data.pixel_shader_circle_inner_shadow->Release();
    if (backend_data.pixel_shader_line_shadow)
        backend_data.pixel_shader_line_shadow->Release();
    if (backend_data.render_target_view)
        backend_data.render_target_view->Release();
    if (backend_data.swap_chain)
        backend_data.swap_chain->Release();
    if (backend_data.device_ctx)
        backend_data.device_ctx->Release();
    if (backend_data.device)
        backend_data.device->Release();
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

    ID3DBlob* vs_blob = nullptr;
    constexpr uint8_t shader_data[] = {
#embed "shaders.hlsl"
    };

    // Compiling and creating vertex shader
    result = CompileShader(shader_data, sizeof(shader_data), "VS", "vs_4_0", &vs_blob);

    if (FAILED(result))
    {
        LogFatal("Couldn't compile vertex shader, code: {:#x}", result);
        return result;
    }
    LogDebug("Vertex shader compiled");
    result = backend_data.device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr,
                                                     &backend_data.vertex_shader);
    if (FAILED(result))
    {
        LogFatal("Couldn't create vertex shader, code: {:#x}", result);
        vs_blob->Release();
        return result;
    }
    LogDebug("Vertex shader created");

    // Creating input layout
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

    if (FAILED(result))
    {
        LogFatal("Couldn't create input layout, code: {:#x}", result);
        return result;
    }
    backend_data.device_ctx->IASetInputLayout(backend_data.vertex_layout);

    // Compiling and creating pixel shaders
    ID3DBlob* ps_blob = nullptr;

    std::unordered_map<const char*, ID3D11PixelShader**> shaders = {
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
        {"PSLineShadow", &backend_data.pixel_shader_line_shadow}};

    LogDebug("Pixel shaders ({}):", shaders.size());
    LogPushTab();

    for (auto& it : shaders)
    {
        result = CompileShader(shader_data, sizeof(shader_data), it.first, "ps_4_0", &ps_blob);
        if (FAILED(result))
        {
            LogFatal("Couldn't compile \"{}\", code: {:#x}", result, it.first);
            return result;
        }
        result = backend_data.device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr,
                                                        it.second);

        if (FAILED(result))
        {
            LogFatal("Couldn't create \"{}\", code: {:#x}", result, it.first);
            return result;
        }

        ps_blob->Release();
        LogDebug("\"{}\" success", it.first);
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

    RECT rect;


    this->ResizeViewport();

    LogPopTab();

    return result;
}

void Renderer::PreRender()
{
    this->__pre_render_impl();
}

void Renderer::Render()
{
    __frame_count += 1;

    float ClearColor[4] = {0.35f, 0.35f, 0.5f, 1.0f};
    float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    backend_data.device_ctx->OMSetRenderTargets(1, &backend_data.render_target_view, nullptr);
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

    if (!backend_data.index_buffer.ptr || backend_data.index_buffer.size < __indicies.size())
    {
        if (backend_data.index_buffer.ptr)
            backend_data.index_buffer.ptr->Release();

        backend_data.index_buffer.size = __indicies.size() * sizeof(IndexSize_t) + 4096;
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
    std::memcpy(index_dest, __indicies.data(), __indicies.size() * sizeof(IndexSize_t));

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

    for (size_t chunk_i = 0; chunk_i < __indicies_chunks.size(); ++chunk_i)
    {
        auto& chunk = __indicies_chunks[chunk_i];

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

    __vertices.clear();
    __indicies.clear();
    __indicies_chunks.clear();
    __scissor_queue.clear();

#ifdef IWA_SR_AUTO_SWAP
    backend_data.swap_chain->Present(0, 0);
#endif

    this->__post_render_impl();
}

void Renderer::ResizeViewport()
{
    if (backend_data.device != nullptr)
    {
        RECT rect{};
        GetClientRect(backend_data.hwnd, &rect);

        float width = rect.right - rect.left;
        float height = rect.bottom - rect.top;

        this->__resize_viewport_impl(WinToIwaRect(rect));

        backend_data.device_ctx->OMSetRenderTargets(0, nullptr, nullptr);
        if (backend_data.render_target_view)
        {
            backend_data.render_target_view->Release();
            backend_data.render_target_view = nullptr;
        }

        HRESULT hr = backend_data.swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (SUCCEEDED(hr))
        {
            ID3D11Texture2D* pBackBuffer = nullptr;
            backend_data.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

            backend_data.device->CreateRenderTargetView(pBackBuffer, nullptr, &backend_data.render_target_view);
            pBackBuffer->Release();

            backend_data.viewport.Width = static_cast<float>(width);
            backend_data.viewport.Height = static_cast<float>(height);
            backend_data.viewport.MinDepth = 0.0f;
            backend_data.viewport.MaxDepth = 1.0f;
            backend_data.viewport.TopLeftX = 0;
            backend_data.viewport.TopLeftY = 0;
            backend_data.device_ctx->RSSetViewports(1, &backend_data.viewport);
        }

        backend_data.device_ctx->OMSetRenderTargets(1, &backend_data.render_target_view, nullptr);
    }
}
