#pragma once

#include "sdk/abstractions.hpp"
#include "sdk/inputs.hpp"
#include "sdk/utils/logger.hpp"
#include "sdk/utils/signal.hpp"
#include "sdk/utils/singleton.hpp"
#include "sdk/math.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <inplace_vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Iwa
{
    namespace SDK
    {
        struct Vertex_t
        {
            Vec2_t pos = {0,0};
            ColorU32_t color = 0;
            Vec2_t uv = {0,0};
        };

        enum class IndexChunkType : char8_t
        {
            BASIC, TEXTURED
        };

        using IndexSize_t = uint32_t;

        struct RenderableRect_t
        {
            Vec2_t min;
            Vec2_t max;
            ColorU32_t color;
        };

        struct IndexChunk_t
        {
            IndexSize_t index = 0;
            IndexChunkType shader_type = IndexChunkType::BASIC;
            void* data = nullptr;
        };

        template<typename TTexture, typename TResource>
        class AbstractTexture : public AIDEntity<TTexture>
        {
        public:
            AbstractTexture(const std::string& id) : AIDEntity<TTexture>(id) {}
            virtual bool Load(const std::vector<uint8_t>& data) const = 0;
            virtual bool Load(const std::filesystem::path& path) const = 0;
            constexpr TResource* getResource() { return this->__resource; }
            const bool isLoaded() { return this->__resource != nullptr; }

        protected:
            mutable TResource* __resource = nullptr;
            mutable Vec2_t __size = {-1.0f,-1.0f};
            // @todo: colors
        };

        template <typename TFont, typename TResource>
        class AbstractFont : public AbstractTexture<TFont, TResource>
        {
        public:
            AbstractFont(const std::string& id) : Iwa::SDK::AbstractTexture<TFont, TResource>(id) {}
            virtual bool LoadFromSystem(const std::filesystem::path& path) const = 0;
        };

        template <typename TRenderContext, typename TResource>
        class AbstractRenderContext : IRenderable, public AIDEntity<TRenderContext>
        {
        public:
            AbstractRenderContext(const std::string& id) : AIDEntity<TRenderContext>(id) { this->__id = id; }
            ~AbstractRenderContext() { reinterpret_cast<TRenderContext*>(this)->Cleanup(); }
            virtual void DrawRect(const RenderableRect_t& rect) = 0;
            virtual void DrawTexturedRect(const RenderableRect_t& rect, TResource* resource) = 0;
            // virtual void DrawLine(const RenderableRect &rect) = 0;
        protected:
            std::vector<Vertex_t> __vertices = {};
            std::vector<IndexSize_t> __indicies = {};
            std::vector<IndexChunk_t> __indicies_chunks = {};
        };

        template <typename TRenderer, typename TBackendData, typename TRenderContext>
        class AbstractRenderer : public Singleton<TRenderer>
        {
        public:
            // Override in derived classes
            static constexpr const char* getID()
            {
                static_assert(TRenderer::getID == AbstractRenderer<TRenderer, TBackendData, TRenderContext>::getID(),
                              "You must override method \"static constexpr const char* getID()\"");
                return "unnamed-renderer";
            }

            using ContextsMap = std::unordered_map<std::string, TRenderContext*>;

            const ContextsMap& getContexts() { return this->__contexts; }
            const TBackendData& getBackendData() { return this->__bd; };
            constexpr const TBackendData* getBackendDataptr() { return &this->__bd; };

            virtual void Render() = 0;
            void AddRenderContext(TRenderContext* ctx)
            {
                const std::string& id = ctx->getID();
                if (this->__contexts.contains(id))
                    LOGW("Rewriting context with id \"%s\".", id.data());

                this->__contexts.emplace(id, ctx);
            }

        protected:
            ContextsMap __contexts{};
            TBackendData __bd{};
            uint64_t __frame_count = 0;
        };
    } // namespace SDK
} // namespace Iwa
