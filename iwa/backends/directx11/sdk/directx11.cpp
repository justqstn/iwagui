#include "sdk/directx11.hpp"
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

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb_truetype.h"

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
using namespace Iwa::SDK;

namespace
{
    HRESULT CompileShader(const unsigned char* shader_data, size_t shader_size, LPCSTR szEntryPoint,
                          LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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

Texture::Texture(const std::string& id) : Iwa::SDK::AbstractTexture<Texture, ID3D11ShaderResourceView>::AbstractTexture(id) {}

bool Texture::Load(const std::vector<uint8_t>& bytes) const
{
    HRESULT result = S_OK;
    auto& backend_data = Renderer::getInstance()->getBackendData();

    int image_width, image_height = 0;
    uint8_t* image_data = stbi_load_from_memory(const_cast<const uint8_t*>(bytes.data()), bytes.size(), &image_width, &image_height, nullptr, 4);

    this->__size.x = image_width;
    this->__size.y = image_height;
    if (image_data == nullptr)
    {
        LOGE("Couldn't load image_data: stbi_load_from_memory returned nullptr.");
        return false;
    }

    D3D11_TEXTURE2D_DESC desc = {};
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = static_cast<int>(this->__size.x);
    desc.Height = static_cast<int>(this->__size.y);
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D *texture_ptr = nullptr;
    D3D11_SUBRESOURCE_DATA sub_resource = {};
    ZeroMemory(&sub_resource, sizeof(sub_resource));
    sub_resource.pSysMem = image_data;
    sub_resource.SysMemPitch = desc.Width * 4;
    sub_resource.SysMemSlicePitch = 0;
    result = backend_data.device->CreateTexture2D(&desc, &sub_resource, &texture_ptr);
    if (FAILED(result))
    {
        LOGE("Couldn't create Texture2D, code: 0x%x", result);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = desc.MipLevels;
    srv_desc.Texture2D.MostDetailedMip = 0;

    result = backend_data.device->CreateShaderResourceView(texture_ptr, &srv_desc, &this->__resource);

    texture_ptr->Release();
    if (FAILED(result))
    {
        LOGE("Couldn't create shader resource view, code: 0x%x", result);
    }

    stbi_image_free(image_data);

    return SUCCEEDED(result);
}

bool Texture::Load(const std::filesystem::path& path) const
{
    assert(not std::filesystem::is_regular_file(path) && std::format("File {} is not existing.", path.string()).data());

    std::ifstream file_input(path);
    std::vector<uint8_t> file_data((std::istreambuf_iterator<char>(file_input)), std::istreambuf_iterator<char>());

    return this->Load(file_data);
}

Font::Font(const std::string& id) : SDK::AbstractFont<Font, ID3D11ShaderResourceView>(id) {}

bool Font::Load(const std::vector<uint8_t>& bytes) const
{
    auto& backend_data = SDK::Renderer::getInstance()->getBackendData();
    std::array<stbtt_bakedchar, 96> baked_chars;
    std::vector<uint8_t> atlas_pixels(Font::ATLAS_SIZE.x * Font::ATLAS_SIZE.y);

    int code = stbtt_BakeFontBitmap(bytes.data(), 0, 32.0f, atlas_pixels.data(), Font::ATLAS_SIZE.x, Font::ATLAS_SIZE.y, 32, 96, baked_chars.data());

    ID3D11Texture2D* texture = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = Font::ATLAS_SIZE.x;
    desc.Height = Font::ATLAS_SIZE.y;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;


    D3D11_SUBRESOURCE_DATA subres = {};
    subres.pSysMem = atlas_pixels.data();
    subres.SysMemPitch = Font::ATLAS_SIZE.x;


    backend_data.device->CreateTexture2D(&desc, &subres, &texture);
    backend_data.device->CreateShaderResourceView(texture, nullptr, &srv);

    return true;
}

bool Font::Load(const std::filesystem::path& path) const
{
    return true;
}

bool Font::LoadFromSystem(const std::filesystem::path& path) const
{
    return true;
}

RenderContext::RenderContext(const std::string& id)
    : AbstractRenderContext(id)
{
    LOG(Renderer::LOG_CHANNEL_RENDERER, Iwa::Logger::COLOR_DARK_MAGENTA, "RenderContext::RenderContext(0x%x)", id.data())
}

void RenderContext::DrawRect(const RenderableRect_t& rect)
{
    Vec2_t nrm_min = Vec2_t { rect.min.x * 2 - 1, 1 - rect.min.y * 2 };
    Vec2_t nrm_max = Vec2_t { rect.max.x * 2 - 1, 1 - rect.max.y * 2 };

    uint32_t start_index = this->__vertices.size();

    this->__vertices.emplace_back(Vertex_t{nrm_min, rect.color, {0,0}});
    this->__vertices.emplace_back(Vertex_t{Vec2_t{nrm_max.x, nrm_min.y}, rect.color, {1,0}});
    this->__vertices.emplace_back(Vertex_t{nrm_max, rect.color, {1,1}});
    this->__vertices.emplace_back(Vertex_t{Vec2_t{nrm_min.x, nrm_max.y}, rect.color, {0,1}});

    this->__indicies.emplace_back(start_index);
    this->__indicies.emplace_back(start_index + 1);
    this->__indicies.emplace_back(start_index + 2);
    this->__indicies.emplace_back(start_index + 2);
    this->__indicies.emplace_back(start_index + 3);
    this->__indicies.emplace_back(start_index);

    if (this->__indicies_chunks.size() > 0)
    {
        auto& index_pair = this->__indicies_chunks[this->__indicies_chunks.size()-1];
        if (index_pair.shader_type != IndexChunkType::BASIC)
        {
            this->__indicies_chunks.emplace_back(
                IndexChunk_t{.index = index_pair.index + 6, .shader_type = IndexChunkType::BASIC, .data = nullptr});
        }
        else
        {
            index_pair.index += 6;
        }
    }
    else
        this->__indicies_chunks.emplace_back(
            IndexChunk_t{.index = 6, .shader_type = IndexChunkType::BASIC, .data = nullptr});
}

void RenderContext::DrawTexturedRect(const RenderableRect_t& rect, ID3D11ShaderResourceView* resource)
{
    Vec2_t nrm_min = Vec2_t { rect.min.x * 2 - 1, 1 - rect.min.y * 2 };
    Vec2_t nrm_max = Vec2_t { rect.max.x * 2 - 1, 1 - rect.max.y * 2 };

    uint32_t start_index = this->__vertices.size();
    this->__vertices.emplace_back(Vertex_t{nrm_min, rect.color, {0,0}});
    this->__vertices.emplace_back(Vertex_t{Vec2_t{nrm_max.x, nrm_min.y}, rect.color, {1,0}});
    this->__vertices.emplace_back(Vertex_t{nrm_max, rect.color, {1,1}});
    this->__vertices.emplace_back(Vertex_t{Vec2_t{nrm_min.x, nrm_max.y}, rect.color, {0,1}});

    this->__indicies.emplace_back(start_index);
    this->__indicies.emplace_back(start_index + 1);
    this->__indicies.emplace_back(start_index + 2);
    this->__indicies.emplace_back(start_index + 2);
    this->__indicies.emplace_back(start_index + 3);
    this->__indicies.emplace_back(start_index);

    if (this->__indicies_chunks.size() > 0)
    {
        auto& index_pair = this->__indicies_chunks[this->__indicies_chunks.size()-1];
        if (index_pair.shader_type != IndexChunkType::TEXTURED || index_pair.data != resource)
        {
            this->__indicies_chunks.emplace_back(
                IndexChunk_t{.index = index_pair.index + 6, .shader_type = IndexChunkType::TEXTURED, .data = resource});
        }
        else
        {
            index_pair.index += 6;
        }
    }
    else
        this->__indicies_chunks.emplace_back(
            IndexChunk_t{.index = 6, .shader_type = IndexChunkType::TEXTURED, .data = resource});
}

void RenderContext::Render()
{
    auto backend_data = Renderer::getInstance()->getBackendData();

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
            LOGE("Failed resizing vertex buffer, code: 0x%x", result);
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
            LOGE("Failed resizing index buffer, code: 0x%x", result);
    }

    D3D11_MAPPED_SUBRESOURCE vertex_resource, index_resource;

    result = backend_data.device_ctx->Map(this->vertex_buffer.ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertex_resource);

    if (FAILED(result))
        LOGE("Failed mapping vertex buffer, code: 0x%x", result);

    result = backend_data.device_ctx->Map(this->index_buffer.ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &index_resource);

    if (FAILED(result))
        LOGE("Failed mapping vertex buffer, code: 0x%x", result);

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
            }
            cur_chtype = chunk.shader_type;
        }

        switch (chunk.shader_type) {
            case IndexChunkType::BASIC:
                break;
            case IndexChunkType::TEXTURED:
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

Renderer::Renderer()
{
    LOGV("Renderer::Renderer");
    Logger::getInstance()->Ignore("FRAMESTATS:V");
}

Renderer::~Renderer()
{
    this->Cleanup();
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
    if (this->__bd.render_target_view)
        this->__bd.render_target_view->Release();
    if (this->__bd.swap_chain)
        this->__bd.swap_chain->Release();
    if (this->__bd.device_ctx)
        this->__bd.device_ctx->Release();
    if (this->__bd.device)
        this->__bd.device->Release();
}

HRESULT Renderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* immediate_context, IDXGISwapChain* swap_chain,
                             ID3D11RenderTargetView* render_target_view)
{
    this->__bd.device = device;
    this->__bd.device_ctx = immediate_context;
    this->__bd.swap_chain = swap_chain;
    this->__bd.render_target_view = render_target_view;

    return Initialize();
}

HRESULT Renderer::Initialize()
{
    LOG(LOG_CHANNEL_RENDERER, Iwa::Logger::COLOR_DARK_MAGENTA, "Initializing renderer with id \"0x%x\"", this->getID());

    assert(this->__bd.device != nullptr && "\"this->__bd.device\" is null");
    assert(this->__bd.device_ctx != nullptr && "\"this->__bd.immediate_context\" is null");
    assert(this->__bd.swap_chain != nullptr && "\"this->__bd.swap_chain\" is null");
    assert(this->__bd.render_target_view != nullptr && "\"this->__bd.render_target_view\" is null");

    HRESULT result = S_OK;

    ID3DBlob* vs_blob = nullptr;
    constexpr unsigned char shader_data[] = {
        #embed "basic_shader.hlsl"
    };

    // Compiling and creating vertex shader
    result = CompileShader(shader_data, sizeof(shader_data), "VS", "vs_4_0", &vs_blob);

    if (FAILED(result))
    {
        LOGE("Couldn't compile vertex shader, code: 0x%x", result)
        return result;
    }
    LOG(LOG_CHANNEL_RENDERER, Iwa::Logger::COLOR_DARK_MAGENTA, "Vertex shader compiled");
    result = this->__bd.device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr,
                                                   &this->__bd.vertex_shader);
    if (FAILED(result))
    {
        LOGE("Couldn't create vertex shader, code: 0x%x", result)
        vs_blob->Release();
        return result;
    }
    LOG(LOG_CHANNEL_RENDERER, Iwa::Logger::COLOR_DARK_MAGENTA, "Vertex shader created");

    // Creating input layout
    constexpr D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetof(Vertex_t, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)offsetof(Vertex_t, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetof(Vertex_t, uv), D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    result = this->__bd.device->CreateInputLayout(layout, ARRAYSIZE(layout), vs_blob->GetBufferPointer(),
                                                  vs_blob->GetBufferSize(), &this->__bd.vertex_layout);
    LOG(LOG_CHANNEL_RENDERER, Iwa::Logger::COLOR_DARK_MAGENTA, "Input layout created");
    vs_blob->Release();

    if (FAILED(result))
    {
        LOGE("Couldn't create input layout, code: 0x%x", result)
        return result;
    }
    this->__bd.device_ctx->IASetInputLayout(this->__bd.vertex_layout);

    // Compiling and creating pixel shader
    ID3DBlob* ps_blob = nullptr;
    result = CompileShader(shader_data, sizeof(shader_data), "PS", "ps_4_0", &ps_blob);
    if (FAILED(result))
    {
        LOGE("Couldn't compile pixel shader, code: 0x%x", result);
        return result;
    }
    LOG(LOG_CHANNEL_RENDERER, Iwa::Logger::COLOR_DARK_MAGENTA, "Pixel shader compiled");

    result = this->__bd.device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr,
                                                  &this->__bd.pixel_shader);
    ps_blob->Release();
    // Compiling and creating textured pixel shader
    result = CompileShader(shader_data, sizeof(shader_data), "PST", "ps_4_0", &ps_blob);
    if (FAILED(result))
    {
        LOGE("Couldn't compile pixel shader, code: 0x%x", result);
        return result;
    }
    LOG(LOG_CHANNEL_RENDERER, Iwa::Logger::COLOR_DARK_MAGENTA, "Textured pixel shader compiled");

    result = this->__bd.device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr,
                                                  &this->__bd.pixel_shader_textured);
    ps_blob->Release();

    if (FAILED(result))
    {
        LOGE("Couldn't create pixel shader, code: 0x%x", result);
        return result;
    }

    LOG(LOG_CHANNEL_RENDERER, Iwa::Logger::COLOR_DARK_MAGENTA, "Pixel shader created");

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
        LOGE("Couldn't create depth stencil state, code: 0x%x", result);
        return result;
    }

    this->__bd.device_ctx->OMSetDepthStencilState(this->__bd.disabled_stencil_state, 1);

    D3D11_BLEND_DESC blend_desc = {};
    blend_desc.RenderTarget[0].BlendEnable = TRUE;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    this->__bd.device->CreateBlendState(&blend_desc, &this->__bd.blend_state);


    D3D11_SAMPLER_DESC sampler_desc = {};
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.MipLODBias = 0.0f;
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.MinLOD = 0.0f;
	sampler_desc.MaxLOD = 0.0f;
	// desc.MinLOD = 0;
	// desc.MaxLOD = 1000.0f;

	result = this->__bd.device->CreateSamplerState(&sampler_desc, &this->__bd.sampler_state);

	if (FAILED(result))
	{
	    LOGE("Couldn't create sampler state, code: 0x%x", result);
	    return result;
	}

    return result;
}

void Renderer::Render()
{
    this->__frame_count += 1;

    LOG(LOG_CHANNEL_FRAMESTATS, Iwa::Logger::COLOR_LIGHT_CYAN, "[%i] Preparing", this->__frame_count);

    float ClearColor[4] = {0.35f, 0.35f, 0.5f, 1.0f};
    float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    this->__bd.device_ctx->OMSetBlendState(this->__bd.blend_state, blendFactor, 0xffffffff);
    this->__bd.device_ctx->ClearRenderTargetView(this->__bd.render_target_view, ClearColor);
    this->__bd.device_ctx->VSSetShader(this->__bd.vertex_shader, nullptr, 0);
    this->__bd.device_ctx->PSSetSamplers(0, 1, &this->__bd.sampler_state);

    LOG(LOG_CHANNEL_FRAMESTATS, Iwa::Logger::COLOR_LIGHT_CYAN, "[%i] Starting rendering Iwa contexts",
        this->__frame_count);

    for (auto& it : this->__contexts)
    {
        auto* id = it.second->getID().data();
        LOG(LOG_CHANNEL_FRAMESTATS, Iwa::Logger::COLOR_LIGHT_CYAN, "[%i] Context 0x%x: rendering...", this->__frame_count,
            id);
        it.second->Render();
        LOG(LOG_CHANNEL_FRAMESTATS, Iwa::Logger::COLOR_LIGHT_CYAN, "[%i] Context 0x%x: success", this->__frame_count, id);
    }

    LOG(LOG_CHANNEL_FRAMESTATS, Iwa::Logger::COLOR_LIGHT_CYAN, "[%i] Presenting... ", this->__frame_count);

#ifdef IWA_SR_AUTO_SWAP
    this->__bd.swap_chain->Present(1, 0);
#endif
}
