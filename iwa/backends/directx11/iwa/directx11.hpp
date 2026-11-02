#pragma once

#include "abstractions/abstractions.hpp"
#include "utils/math.hpp"
#include "abstractions/rendering/renderer.hpp"

#include <cstdint>
#include <d3d11.h>
#include <filesystem>
#include <vector>
#include <windef.h>
#include <winnt.h>

namespace Iwa
{
    namespace __private__
    {
        struct ConstantBuffer_t
        {
            float model_view_projection[4][4];
        };

        struct Buffer_t
        {
            ID3D11Buffer* ptr = nullptr;
            size_t size = 0;
        };

        struct BackendData_t
        {
            Buffer_t vertex_buffer;
            Buffer_t index_buffer;
            D3D11_VIEWPORT viewport;
            ID3D11Device* device = nullptr;
            ID3D11DeviceContext* device_ctx = nullptr;
            IDXGISwapChain* swap_chain = nullptr;
            ID3D11InputLayout* vertex_layout = nullptr;
            ID3D11RenderTargetView* render_target_view = nullptr;
            ID3D11DepthStencilState* disabled_stencil_state = nullptr;
            ID3D11RasterizerState* rasterizer_state = nullptr;
            ID3D11BlendState* blend_state = nullptr;
            ID3D11SamplerState* sampler_state = nullptr;
            ID3D11SamplerState* sampler_state_linear = nullptr;
            ID3D11VertexShader* vertex_shader = nullptr;
            ID3D11PixelShader* pixel_shader = nullptr;
            ID3D11PixelShader* pixel_shader_textured = nullptr;
            ID3D11PixelShader* pixel_shader_text = nullptr;
            ID3D11PixelShader* pixel_shader_text_sdf = nullptr;
            ID3D11PixelShader* pixel_shader_shadow = nullptr;
            ID3D11PixelShader* pixel_shader_inner_shadow = nullptr;
            ID3D11PixelShader* pixel_shader_outline = nullptr;
            ID3D11PixelShader* pixel_shader_circle = nullptr;
            ID3D11PixelShader* pixel_shader_circle_textured = nullptr;
            ID3D11PixelShader* pixel_shader_circle_shadow = nullptr;
            ID3D11PixelShader* pixel_shader_circle_inner_shadow = nullptr;
            ID3D11PixelShader* pixel_shader_line_shadow = nullptr;
            ID3D11Buffer* constant_buffer = nullptr;
            HWND hwnd = nullptr;
        };

    } // namespace __private__
    // forward declarations for my dear hbs :heart:

    class Renderer;
    class RenderContext;

    class Texture : public __private__::AbstractTexture<Texture, Renderer, ID3D11ShaderResourceView>
    {
    public:
        bool Load(const std::span<const uint8_t> data) override;
        bool Load(const std::filesystem::path& path) override;

    protected:
        Texture(const Renderer* renderer, const std::string& id);
        ~Texture();
    };

    class BitmapFont : public __private__::AbstractBitmapFont<BitmapFont, Renderer, ID3D11ShaderResourceView>
    {
    public:
        BitmapFont* Load(const std::span<const uint8_t> data, const std::span<const CharRange_t> ranges) override;
        BitmapFont* Load(const std::filesystem::path& path, const std::span<const CharRange_t> ranges) override;

        // Only Windows
        BitmapFont* LoadFromSystem(const std::filesystem::path& path,
                                   const std::span<const CharRange_t> ranges) override;

    protected:
        ~BitmapFont();
        BitmapFont(const Renderer* renderer, const std::string& id, float scale = 48.0f);
    };

    class DynamicFont : public __private__::AbstractDynamicFont<DynamicFont, Renderer, ID3D11ShaderResourceView>
    {
    public:
        DynamicFont* Load(const std::span<const uint8_t> data, const std::span<const CharRange_t> ranges) override;
        DynamicFont* Load(const std::filesystem::path& path, const std::span<const CharRange_t> ranges) override;

        // Only Windows
        DynamicFont* LoadFromSystem(const std::filesystem::path& path,
                                    const std::span<const CharRange_t> ranges) override;

    protected:
        ~DynamicFont();
        DynamicFont(const Renderer* renderer, const std::string& id, float bake_size = 32.0f);
    };

    class Renderer : public __private__::AbstractRenderer<Renderer, __private__::BackendData_t, Texture, ID3D11ShaderResourceView, BitmapFont, DynamicFont>
    {
    public:
        static constexpr const char* getRendererID() { return "DX11"; }
        static constexpr const char* LOG_CHANNEL_RENDERER = "RENDERER";
        static constexpr size_t DEFAULT_RECT_COUNT = 512;
        static constexpr size_t DEFAULT_INDEX_BUFFER_SIZE = DEFAULT_RECT_COUNT * 6;
        static constexpr size_t DEFAULT_VERTEX_BUFFER_SIZE = DEFAULT_RECT_COUNT * sizeof(__private__::Vertex_t) * 4;

        HRESULT Initialize();
        HRESULT
        Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* immediate_context, IDXGISwapChain* swap_chain,
                   ID3D11RenderTargetView* render_target_view);

        void Render() override;
        void PreRender() override;
        void ResizeViewport() override;

    protected:

        Renderer(const std::string& id);
        ~Renderer();
    };
} // namespace Iwa
