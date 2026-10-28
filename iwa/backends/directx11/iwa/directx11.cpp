#include <unordered_map>
#include <windows.h>
#include <wingdi.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include "thirdparty/stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#include "iwa/directx11.hpp"
#include "sdk/abstractions.hpp"
#include "sdk/math.hpp"
#include "sdk/renderer.hpp"
#include "sdk/utils/logger.hpp"
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
#include <d3dcompiler.h>
#include <d3d11.h>
#include <inplace_vector>
#include <minwindef.h>
#include <ratio>
#include <vector>
#include <winerror.h>
#include <winnt.h>

using namespace Iwa;
using namespace Iwa::Internal;


namespace
{
    HRESULT CompileShader(const uint8_t* shader_data, size_t shader_size, LPCSTR szEntryPoint, LPCSTR szShaderModel,
                          ID3DBlob** ppBlobOut)
    {
        HRESULT result = S_OK;
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
        ID3DBlob* pErrorBlob;

        result = D3DCompile(shader_data, shader_size, nullptr, nullptr, nullptr, szEntryPoint, szShaderModel, dwShaderFlags, 0,
                            ppBlobOut, &pErrorBlob);

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

Texture::Texture(const Renderer* renderer, const std::string& id) : AbstractTexture<Texture, Renderer, ID3D11ShaderResourceView>::AbstractTexture(renderer, id) {}

Texture::~Texture()
{

}

bool Texture::Load(const std::span<const uint8_t> data)
{
    HRESULT result = S_OK;
    auto& backend_data = this->renderer->getBackendData();

    // todo: refactor ts

    int image_width, image_height = 0;
    uint8_t* image_data = stbi_load_from_memory(const_cast<const uint8_t*>(data.data()), data.size(), &image_width, &image_height, nullptr, 4);

    this->__size = Vec2_t(image_width, image_height);
    if (image_data == nullptr)
    {
        LogFatal("Couldn't load image_data: stbi_load_from_memory returned nullptr.");
    }

    ID3D11Texture2D *texture = nullptr;
    D3D11_TEXTURE2D_DESC tex_desc = {};
    tex_desc.Width = static_cast<int>(this->__size.x);
    tex_desc.Height = static_cast<int>(this->__size.y);
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
    result = backend_data.device->CreateTexture2D(&tex_desc, &subres, &texture);
    if (FAILED(result))
    {
        LogFatal("Couldn't create Texture2D, code: {:#x}", result);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = tex_desc.MipLevels;
    srv_desc.Texture2D.MostDetailedMip = 0;

    result = backend_data.device->CreateShaderResourceView(texture, &srv_desc, &this->__resource);
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
    IWA_ASSERT(not std::filesystem::is_regular_file(path) && std::format("File {} is not existing.", path.string()).data());

    std::ifstream file_input(path);
    std::vector<uint8_t> file_data((std::istreambuf_iterator<char>(file_input)), std::istreambuf_iterator<char>());

    return this->Load(file_data);
}

BitmapFont::BitmapFont(const Renderer* renderer, const std::string& id, float scale) : AbstractBitmapFont<BitmapFont, Renderer, ID3D11ShaderResourceView>(renderer, id, scale) {}

BitmapFont* BitmapFont::Load(const std::span<const uint8_t> data, const std::span<const CharRange_t> ranges)
{
    auto pixels = this->__alloc_and_pack_font_bitmap(data, ranges);

    HRESULT result = S_OK;
    auto& backend_data = this->renderer->getBackendData();

    ID3D11Texture2D* texture = nullptr;
    D3D11_TEXTURE2D_DESC tex_desc = {};
    tex_desc.Width = this->__atlas_size.x;
    tex_desc.Height = this->__atlas_size.y;
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Format = DXGI_FORMAT_R8_UNORM;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
    tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subres = {};
    subres.pSysMem = pixels->data();
    subres.SysMemPitch = this->__atlas_size.x;

    result = backend_data.device->CreateTexture2D(&tex_desc, &subres, &texture);

    if (FAILED(result))
    {
        LogFatal("Coulnd't create glyph texture, code: {:#x}", result);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = DXGI_FORMAT_R8_UNORM;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;

    result = backend_data.device->CreateShaderResourceView(texture, &srv_desc, &this->__resource);
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

BitmapFont* BitmapFont::LoadFromSystem(const std::filesystem::path& path)
{
    return this;
}

RenderContext::RenderContext(const Renderer* renderer, const std::string& id) : AbstractRenderContext(renderer, id)
{
    LogDebug("RenderContext::RenderContext({})", id);
}

void RenderContext::Render()
{
    auto backend_data = this->renderer->getBackendData();

    constexpr UINT stride = static_cast<UINT>(sizeof(Vertex_t));
    constexpr UINT vertex_offset = 0;

    HRESULT result = S_OK;

    if (!this->vertex_buffer.ptr || this->vertex_buffer.size < this->__vertices.size())
    {
        if (this->vertex_buffer.ptr)
            this->vertex_buffer.ptr->Release();

        this->vertex_buffer.size = this->__vertices.size() * sizeof(Vertex_t) + 4096;
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = this->__vertices.size() * sizeof(Vertex_t) + 4096;
        desc.MiscFlags = 0;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        result = backend_data.device->CreateBuffer(&desc, nullptr, &this->vertex_buffer.ptr);
        if (FAILED(result))
            LogFatal("Failed resizing vertex buffer, code: {:#x}", result);
    }

    if (!this->index_buffer.ptr || this->index_buffer.size < this->__indicies.size())
    {
        if (this->index_buffer.ptr)
            this->index_buffer.ptr->Release();

        this->index_buffer.size = this->__indicies.size() * sizeof(IndexChunk_t) + 4096;
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = this->index_buffer.size;
        desc.MiscFlags = 0;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT result = backend_data.device->CreateBuffer(&desc, nullptr, &this->index_buffer.ptr);
        if (FAILED(result))
            LogFatal("Failed resizing index buffer, code: {:#x}", result);
    }

    D3D11_MAPPED_SUBRESOURCE vertex_resource, index_resource;

    result = backend_data.device_ctx->Map(this->vertex_buffer.ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertex_resource);

    if (FAILED(result))
        LogFatal("Failed mapping vertex buffer, code: {:#x}", result);

    result = backend_data.device_ctx->Map(this->index_buffer.ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &index_resource);

    if (FAILED(result))
        LogFatal("Failed mapping vertex buffer, code: {:#x}", result);

    Vertex_t* vertex_dest = static_cast<Vertex_t*>(vertex_resource.pData);
    IndexChunk_t* index_dest = static_cast<IndexChunk_t*>(index_resource.pData);

    std::memcpy(vertex_dest, this->__vertices.data(), this->__vertices.size() * sizeof(Vertex_t));
    std::memcpy(index_dest, this->__indicies.data(), this->__indicies.size() * sizeof(IndexSize_t));

    backend_data.device_ctx->Unmap(this->vertex_buffer.ptr, 0);
    backend_data.device_ctx->Unmap(this->index_buffer.ptr, 0);

    backend_data.device_ctx->IASetVertexBuffers(0, 1, &this->vertex_buffer.ptr, &stride, &vertex_offset);
    backend_data.device_ctx->IASetIndexBuffer(this->index_buffer.ptr, DXGI_FORMAT_R32_UINT, 0);

    IndexSize_t index_offset = 0;
    IndexChunkType cur_chtype = IndexChunkType::BASIC;
    backend_data.device_ctx->PSSetShader(backend_data.pixel_shader, nullptr, 0);


    for (auto& chunk : this->__indicies_chunks)
    {
        if (chunk.shader_type != cur_chtype)
        {
            switch (chunk.shader_type) {
                case IndexChunkType::BASIC:
                    backend_data.device_ctx->PSSetShader(backend_data.pixel_shader, nullptr, 0);
                    break;
                case IndexChunkType::TEXTURED:
                    backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_textured, nullptr, 0);
                    break;
                case IndexChunkType::TEXT:
                    backend_data.device_ctx->PSSetShader(backend_data.pixel_shader_text, nullptr, 0);
                    break;
            }
            cur_chtype = chunk.shader_type;
        }

        switch (chunk.shader_type) {
            case IndexChunkType::BASIC:
                break;
            case IndexChunkType::TEXTURED:
            case IndexChunkType::TEXT:
                ID3D11ShaderResourceView* srv = static_cast<ID3D11ShaderResourceView*>(chunk.data);
                backend_data.device_ctx->PSSetShaderResources(0, 1, &srv);
                break;
        }

        backend_data.device_ctx->DrawIndexed(chunk.index, index_offset, 0);
        index_offset = chunk.index;
    }

    this->__vertices.clear();
    this->__indicies.clear();
    this->__indicies_chunks.clear();
}

void RenderContext::Cleanup()
{
    if (this->index_buffer.ptr)
    {
        this->index_buffer.ptr->Release();
        this->index_buffer.ptr = nullptr;
    }

    if (this->vertex_buffer.ptr)
    {
        this->vertex_buffer.ptr->Release();
        this->vertex_buffer.ptr = nullptr;
    }
}

Renderer::Renderer(const std::string& id) : AbstractRenderer<Renderer, BackendData_t, RenderContext>(id)
{

}

void Renderer::Cleanup()
{
    if (this->__bd.disabled_stencil_state)
        this->__bd.disabled_stencil_state->Release();
    if (this->__bd.device_ctx)
        this->__bd.device_ctx->ClearState();
    if (this->__bd.vertex_layout)
        this->__bd.vertex_layout->Release();
    if (this->__bd.vertex_shader)
        this->__bd.vertex_shader->Release();
    if (this->__bd.pixel_shader)
        this->__bd.pixel_shader->Release();
    if (this->__bd.pixel_shader_textured)
        this->__bd.pixel_shader_textured->Release();
    if (this->__bd.pixel_shader_text)
        this->__bd.pixel_shader_text->Release();
    if (this->__bd.render_target_view)
        this->__bd.render_target_view->Release();
    if (this->__bd.swap_chain)
        this->__bd.swap_chain->Release();
    if (this->__bd.device_ctx)
        this->__bd.device_ctx->Release();
    if (this->__bd.device)
        this->__bd.device->Release();
}

HRESULT Renderer::Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* immediate_context, IDXGISwapChain* swap_chain,
                             ID3D11RenderTargetView* render_target_view)
{
    this->__bd.hwnd = hwnd;
    this->__bd.device = device;
    this->__bd.device_ctx = immediate_context;
    this->__bd.swap_chain = swap_chain;
    this->__bd.render_target_view = render_target_view;

    return Initialize();
}

HRESULT Renderer::Initialize()
{
    LogDebug("Initializing renderer \"{}\" with render id \"{}\"", this->getID(), this->getRendererID());
    LogPushTab();
    LogPushTab();

    IWA_ASSERT(this->__bd.device != nullptr && "\"this->__bd.device\" is null");
    IWA_ASSERT(this->__bd.device_ctx != nullptr && "\"this->__bd.immediate_context\" is null");
    IWA_ASSERT(this->__bd.swap_chain != nullptr && "\"this->__bd.swap_chain\" is null");
    IWA_ASSERT(this->__bd.render_target_view != nullptr && "\"this->__bd.render_target_view\" is null");
    IWA_ASSERT(this->__bd.hwnd != nullptr && "\"this->__bd.hwnd\" is null");

    HRESULT result = S_OK;

    ID3DBlob* vs_blob = nullptr;
    constexpr uint8_t shader_data[] = {
        #embed "basic_shader.hlsl"
    };

    // Compiling and creating vertex shader
    result = CompileShader(shader_data, sizeof(shader_data), "VS", "vs_4_0", &vs_blob);

    if (FAILED(result))
    {
        LogFatal("Couldn't compile vertex shader, code: {:#x}", result);
        return result;
    }
    LogDebug("Vertex shader compiled");
    result = this->__bd.device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr,
                                                   &this->__bd.vertex_shader);
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
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetof(Vertex_t, uv), D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    result = this->__bd.device->CreateInputLayout(layout, ARRAYSIZE(layout), vs_blob->GetBufferPointer(),
                                                  vs_blob->GetBufferSize(), &this->__bd.vertex_layout);
    LogDebug("Input layout created");
    vs_blob->Release();

    if (FAILED(result))
    {
        LogFatal("Couldn't create input layout, code: {:#x}", result);
        return result;
    }
    this->__bd.device_ctx->IASetInputLayout(this->__bd.vertex_layout);

    // Compiling and creating pixel shaders
    ID3DBlob* ps_blob = nullptr;

    std::unordered_map<const char*, ID3D11PixelShader**> shaders = {
        {"PS",          &this->__bd.pixel_shader},
        {"PSTextured",  &this->__bd.pixel_shader_textured},
        {"PSText",      &this->__bd.pixel_shader_text}
    };

    LogDebug("Pixel shaders ({}):", shaders.size());
    LogPushTab();
    LogPushTab();

    for (auto& it : shaders)
    {
        result = CompileShader(shader_data, sizeof(shader_data), it.first, "ps_4_0", &ps_blob);
        if (FAILED(result))
        {
            LogFatal("Couldn't compile \"{}\", code: {:#x}", result, it.first);
            return result;
        }
        result = this->__bd.device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, it.second);

        if (FAILED(result))
        {
            LogFatal("Couldn't create \"{}\", code: {:#x}", result, it.first);
            return result;
        }

        ps_blob->Release();
        LogDebug("\"{}\" success", it.first);
    }

    LogPopTab();
    LogPopTab();
    LogDebug("Pixel shaders success");

    this->__bd.device_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

    result = this->__bd.device->CreateDepthStencilState(&stencil_desc, &this->__bd.disabled_stencil_state);
    if (FAILED(result))
    {
        LogFatal("Couldn't create depth stencil state, code: {:#x}", result);
        return result;
    }

    this->__bd.device_ctx->OMSetDepthStencilState(this->__bd.disabled_stencil_state, 1);

    D3D11_BLEND_DESC blend_desc = {};
    blend_desc.RenderTarget[0].BlendEnable = TRUE;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA; // D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    this->__bd.device->CreateBlendState(&blend_desc, &this->__bd.blend_state);


    D3D11_SAMPLER_DESC sampler_desc = {};
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.MipLODBias = 0.0f;
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.MinLOD = 0.0f;
	sampler_desc.MaxLOD = 0.0f;

	result = this->__bd.device->CreateSamplerState(&sampler_desc, &this->__bd.sampler_state);

	if (FAILED(result))
	{
	    LogFatal("Couldn't create sampler state, code: {:#x}", result);
	    return result;
	}

    D3D11_BUFFER_DESC constant_buffer_desc{};
    constant_buffer_desc.ByteWidth = sizeof(ConstantBuffer_t);
    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_buffer_desc.MiscFlags = 0;

    result = this->__bd.device->CreateBuffer(&constant_buffer_desc, nullptr, &this->__bd.constant_buffer);

    if (FAILED(result))
    {
        LogFatal("Couldn't create constant buffer, code: {:#x}", result);
    }

    LogPopTab();
    LogPopTab();

    return result;
}

void Renderer::PreRender()
{
    RECT rect{};
    GetClientRect(this->__bd.hwnd, &rect);

    D3D11_MAPPED_SUBRESOURCE const_buf_resource;

    if (SUCCEEDED(this->__bd.device_ctx->Map(this->__bd.constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &const_buf_resource)))
    {
        auto constant_buffer = static_cast<ConstantBuffer_t*>(const_buf_resource.pData);

        float left = rect.left;
        float reft = rect.right;
        float top = rect.top;
        float bottom = rect.bottom;

        float model_view_projection[4][4] = {
            {2.0f / (reft - left),             0.0f,                               0.0f, 0.0f},
            {0.0f,                             2.0f / (top - bottom),              0.0f, 0.0f},
            {0.0f,                             0.0f,                               0.5f, 0.0f},
            {(reft + left) / (left - reft),    (top + bottom) / (bottom - top),    0.5f, 1.0f},
        };

        std::memcpy(&constant_buffer->model_view_projection, model_view_projection, sizeof(model_view_projection));
        this->__bd.device_ctx->Unmap(this->__bd.constant_buffer, 0);
    }
}

void Renderer::Render()
{
    this->__frame_count += 1;

    float ClearColor[4] = {0.35f, 0.35f, 0.5f, 1.0f};
    float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    this->__bd.device_ctx->OMSetBlendState(this->__bd.blend_state, blendFactor, 0xffffffff);
    this->__bd.device_ctx->ClearRenderTargetView(this->__bd.render_target_view, ClearColor);
    this->__bd.device_ctx->VSSetShader(this->__bd.vertex_shader, nullptr, 0);
    this->__bd.device_ctx->VSSetConstantBuffers(0, 1, &this->__bd.constant_buffer);
    this->__bd.device_ctx->PSSetSamplers(0, 1, &this->__bd.sampler_state);

    for (auto& it : this->__contexts)
    {
        auto* id = it.second->getID().data();
        it.second->Render();
    }

#ifdef IWA_SR_AUTO_SWAP
    this->__bd.swap_chain->Present(1, 0);
#endif
}

void Renderer::ResizeViewport(Vec2_t size)
{
    this->AbstractRenderer::ResizeViewport(size);

    auto [width, height] = size;

    if (this->__bd.device != nullptr)
    {
        this->__bd.device_ctx->OMSetRenderTargets(0, nullptr, nullptr);
        if (this->__bd.render_target_view)
        {
            this->__bd.render_target_view->Release();
            this->__bd.render_target_view = nullptr;
        }

        HRESULT hr = this->__bd.swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (SUCCEEDED(hr))
        {
            ID3D11Texture2D* pBackBuffer = nullptr;
            this->__bd.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

            this->__bd.device->CreateRenderTargetView(pBackBuffer, nullptr, &this->__bd.render_target_view);
            pBackBuffer->Release();

            this->__bd.viewport.Width = static_cast<float>(width);
            this->__bd.viewport.Height = static_cast<float>(height);
            this->__bd.viewport.MinDepth = 0.0f;
            this->__bd.viewport.MaxDepth = 1.0f;
            this->__bd.viewport.TopLeftX = 0;
            this->__bd.viewport.TopLeftY = 0;
            this->__bd.device_ctx->RSSetViewports(1, &this->__bd.viewport);
        }

        this->__bd.device_ctx->OMSetRenderTargets(1, &this->__bd.render_target_view, nullptr);
    }
}
