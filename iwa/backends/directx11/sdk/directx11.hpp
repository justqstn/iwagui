#pragma once

#include "sdk/abstractions.hpp"
#include "sdk/math.hpp"
#include "sdk/renderer.hpp"


#include <cstdint>
#include <d3d11.h>
#include <filesystem>
#include <vector>
#include <windef.h>
#include <winnt.h>

namespace Iwa
{
        struct BackendData_t
        {
            D3D11_VIEWPORT viewport;
            ID3D11Device* device = nullptr;
            ID3D11DeviceContext* device_ctx = nullptr;
            IDXGISwapChain* swap_chain = nullptr;
            ID3D11InputLayout* vertex_layout = nullptr;
            ID3D11RenderTargetView* render_target_view = nullptr;
            ID3D11DepthStencilState* disabled_stencil_state = nullptr;
            ID3D11BlendState* blend_state = nullptr;
            ID3D11SamplerState* sampler_state = nullptr;
            ID3D11VertexShader* vertex_shader = nullptr;
            ID3D11PixelShader* pixel_shader = nullptr;
            ID3D11PixelShader* pixel_shader_textured = nullptr;
            ID3D11PixelShader* pixel_shader_text = nullptr;
            ID3D11Buffer* constant_buffer = nullptr;
            HWND hwnd = nullptr;
        };

        struct ConstantBuffer_t
        {
            float model_view_projection[4][4];
        };

        struct Buffer_t
        {
            ID3D11Buffer* ptr = nullptr;
            size_t size = 0;
        };

        // forward declarations for my dear hbs :heart:

        class Renderer;
        class RenderContext;

        class Texture : public Internal::AbstractTexture<Texture, Renderer, ID3D11ShaderResourceView>
        {
        public:

            bool Load(const std::span<const uint8_t> data) override;
            bool Load(const std::filesystem::path& path) override;
        protected:
            Texture(const Renderer* renderer, const std::string& id);
            ~Texture();
        };

        class BitmapFont : public Internal::AbstractBitmapFont<BitmapFont, Renderer, ID3D11ShaderResourceView>
        {
        public:
            BitmapFont* Load(const std::span<const uint8_t> data, const std::span<const CharRange_t> ranges) override;
            BitmapFont* Load(const std::filesystem::path& path, const std::span<const CharRange_t> ranges) override;

            // Only Windows
            BitmapFont* LoadFromSystem(const std::filesystem::path& path) override;
        protected:
            ~BitmapFont();
            BitmapFont(const Renderer* renderer, const std::string& id, float scale = 48.0f);
        };

        class RenderContext
            : public Internal::AbstractRenderContext<RenderContext, Renderer, Texture, ID3D11ShaderResourceView, BitmapFont>
        {
        public:
            Buffer_t vertex_buffer;
            Buffer_t index_buffer;
            ID3D11RenderTargetView* render_target_view = nullptr;

            constexpr static size_t DEFAULT_RECT_COUNT = 512;
            constexpr static size_t DEFAULT_INDEX_BUFFER_SIZE = DEFAULT_RECT_COUNT * 6;
            constexpr static size_t DEFAULT_VERTEX_BUFFER_SIZE = DEFAULT_RECT_COUNT * sizeof(Internal::Vertex_t) * 4;



            bool isInitialized();
            void Render() override;
            void Cleanup() override;
        protected:
            RenderContext(const Renderer* renderer, const std::string& id);
        private:
            bool __initialized = false;
        };

        class Renderer : public Internal::AbstractRenderer<Renderer, BackendData_t, RenderContext>
        {
        public:
            static constexpr const char* getRendererID() { return "DX11"; }
            static constexpr const char* LOG_CHANNEL_RENDERER = "RENDERER";

            HRESULT Initialize();
            HRESULT
            Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* immediate_context,
                       IDXGISwapChain* swap_chain, ID3D11RenderTargetView* render_target_view);
            void Cleanup() override;
            void Render() override;
            void PreRender() override;
            void ResizeViewport(Vec2_t size) override;
        protected:
            Renderer(const std::string& id);
        };
} // namespace Iwa
