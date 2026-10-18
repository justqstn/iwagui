#pragma once

#include "sdk/utils/singleton.hpp"
#include "sdk/renderer.hpp"
#include "sdk/math.hpp"

#include <cstdint>
#include <d3d11.h>
#include <filesystem>
#include <vector>
#include <winnt.h>

namespace Iwa
{
    namespace SDK
    {
        struct BackendData_t
        {
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
        };

        struct Buffer_t
        {
            ID3D11Buffer* ptr = nullptr;
            size_t size = 0;
        };

        class Texture final : public AbstractTexture<Texture, ID3D11ShaderResourceView>
        {
        public:
            Texture(const std::string& id);
            bool Load(const std::vector<uint8_t>& bytes) const override;
            bool Load(const std::filesystem::path& path) const override;
        };

        class Font final : public AbstractFont<Font, ID3D11ShaderResourceView>
        {
        public:
            constexpr static Vec2_t ATLAS_SIZE = {512,512};
            Font(const std::string& id);

            bool Load(const std::vector<uint8_t>& bytes) const override;
            bool Load(const std::filesystem::path& path) const override;
            bool LoadFromSystem(const std::filesystem::path& path) const override;
        };

        class RenderContext final : public AbstractRenderContext<RenderContext, ID3D11ShaderResourceView>
        {
        public:
            Buffer_t vertex_buffer;
            Buffer_t index_buffer;
            ID3D11RenderTargetView* render_target_view = nullptr;

            constexpr static size_t DEFAULT_RECT_COUNT = 512;
            constexpr static size_t DEFAULT_INDEX_BUFFER_SIZE = DEFAULT_RECT_COUNT * 6;
            constexpr static size_t DEFAULT_VERTEX_BUFFER_SIZE = DEFAULT_RECT_COUNT * sizeof(Vertex_t) * 4;

            RenderContext(const std::string& id);

            bool isInitialized();
            void DrawRect(const RenderableRect_t& rect) override;
            void DrawTexturedRect(const RenderableRect_t& rect, ID3D11ShaderResourceView* resource) override;
            void Render() override;
            void Cleanup() override;

        private:
            bool __initialized = false;
        };

        class Renderer final : public AbstractRenderer<Renderer, BackendData_t, RenderContext>
        {
        public:
            Renderer();
            ~Renderer() override;

            static constexpr const char* getID() { return "Direct3D 11 Renderer"; }

            static constexpr const char* LOG_CHANNEL_RENDERER = "RENDERER";
            static constexpr const char* LOG_CHANNEL_FRAMETIME = "FRAMETIME";
            static constexpr const char* LOG_CHANNEL_FRAMESTATS = "FRAMESTATS:V";

            HRESULT Initialize();
            HRESULT
            Initialize(ID3D11Device* device, ID3D11DeviceContext* immediate_context, IDXGISwapChain* swap_chain,
                       ID3D11RenderTargetView* render_target_view);
            void Cleanup();
            void Render() override;
        };
    } // namespace SDK
} // namespace Iwa
