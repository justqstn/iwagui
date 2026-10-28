#pragma once
#include "thirdparty/stb_truetype.h"

#include "sdk/abstractions.hpp"
#include "sdk/inputs.hpp"
#include "sdk/math.hpp"
#include "sdk/utils/logger.hpp"
#include "sdk/utils/signal.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <inplace_vector>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Iwa
{
    struct CharRange_t
    {
        int first_char;
        int char_count;

        static CharRange_t ASCII() { return {.first_char = 32, .char_count = 95}; }
        static CharRange_t Cyrillic() { return {.first_char = 0x0400, .char_count = 0x04FF - 0x0400}; }
    };

    namespace Internal
    {
        struct Vertex_t
        {
            Vec2_t pos = {0, 0};
            ColorU32_t color = 0;
            Vec2_t uv = {0, 0};
        };

        enum class IndexChunkType : char8_t
        {
            BASIC,
            TEXTURED,
            TEXT
        };

        using IndexSize_t = uint32_t;

        struct RenderableRect_t
        {
            Rect_t rect;
            ColorU32_t color;
        };

        struct IndexChunk_t
        {
            IndexSize_t index = 0;
            IndexChunkType shader_type = IndexChunkType::BASIC;
            void* data = nullptr;
        };

        struct Charset_t
        {
            std::vector<stbtt_packedchar> vec;
            int first_char;
        };

        template <typename TTexture, typename TRenderer, typename TResource>
        class AbstractTexture : public AIDEntity<TTexture>,
                                public HasRenderer<TRenderer>,
                                public HasResource<TResource>,
                                public HasAllocWrapper<TTexture, const TRenderer*, const std::string>
        {
        public:

            virtual bool Load(const std::span<const uint8_t> data) = 0;
            virtual bool Load(const std::filesystem::path& path) = 0;

        protected:
            AbstractTexture(const TRenderer* renderer, const std::string& id)
                : Internal::AIDEntity<TTexture>(id), Internal::HasRenderer<TRenderer>(renderer)
            {
            }

            virtual ~AbstractTexture() {}

            Vec2_t __size = {-1.0f, -1.0f};
        };

        template <typename TFont, typename TRenderer, typename TResource>
        class AbstractFont : public AIDEntity<TFont>,
                             public HasRenderer<TRenderer>,
                             public HasResource<TResource>,
                             public HasAllocWrapper<TFont, const TRenderer*, const std::string&, float>
        {
        public:

            TFont* setAtlasSize(Vec2_t size) { this->__atlas_size = size; }
            Vec2_t getAtlasSize() const { return this->__atlas_size; }

            virtual TFont* LoadFromSystem(const std::filesystem::path& path) = 0;
            virtual TFont* Load(const std::span<const uint8_t> data, const std::span<const CharRange_t> ranges) = 0;
            virtual TFont* Load(const std::filesystem::path& path, const std::span<const CharRange_t> ranges) = 0;

            float getScale() const { return this->__scale; }
        protected:
            Vec2_t __atlas_size;
            float __scale;

            AbstractFont(const TRenderer* renderer, const std::string& id, float scale = 48.0f)
                : AIDEntity<TFont>(id), HasRenderer<TRenderer>(renderer), __scale(scale), __atlas_size({1024,1024})
            {
            }
            ~AbstractFont() {}
        };

        template <typename TFont, typename TRenderer, typename TResource>
        class AbstractBitmapFont : public AbstractFont<TFont, TRenderer, TResource>
        {
        public:
            // ~AbstractBitmapFont() {}

            const Charset_t& getCharset(char32_t codepoint) const
            {
                return *std::find_if(
                    this->__charsets.rbegin(),
                    this->__charsets.rend(),
                    [codepoint](const Charset_t& c) { return codepoint >= c.first_char; }
                );
            }

        protected:
            std::vector<Charset_t> __charsets;

            // @warning: This method allocated vector, delete pointer after calling
            std::vector<uint8_t>* __alloc_and_pack_font_bitmap(const std::span<const uint8_t>& data,
                                                               const std::span<const CharRange_t> char_ranges)
            {
                auto pixels = new std::vector<uint8_t>(this->__atlas_size.x * this->__atlas_size.y);

                stbtt_pack_context pack_ctx{};

                if (!stbtt_PackBegin(&pack_ctx, pixels->data(), this->__atlas_size.x, this->__atlas_size.y, 0, 2, nullptr))
                {
                    LogFatal("Couldn't load font: \"{}\" stbtt_PackBegin() failed!", this->getID());
                }

                stbtt_PackSetOversampling(&pack_ctx, 1, 1);

                std::vector<stbtt_pack_range> ranges;
                this->__charsets.resize(char_ranges.size());

                for (size_t i = 0; i < char_ranges.size(); i++)
                {
                    auto& char_range = char_ranges[i];
                    auto& charset = this->__charsets[i];

                    charset.first_char = char_range.first_char;
                    charset.vec.reserve(char_range.char_count);

                    ranges.emplace_back(stbtt_pack_range{.font_size = this->__scale,
                                                         .first_unicode_codepoint_in_range = char_range.first_char,
                                                         .num_chars = char_range.char_count,
                                                         .chardata_for_range = charset.vec.data()});
                }

                std::sort(this->__charsets.begin(), this->__charsets.end(),
                          [](const Charset_t& a, const Charset_t& b) { return a.first_char < b.first_char; });

                if (!stbtt_PackFontRanges(&pack_ctx, data.data(), 0, ranges.data(), ranges.size()))
                {
                    LogFatal("Couldn't load font: \"{}\" stbtt_PackFontRanges() failed!", this->getID());
                }
                stbtt_PackEnd(&pack_ctx);

                return pixels;
            }

            AbstractBitmapFont(const TRenderer* renderer, const std::string& id, float scale = 48.0f)
                : AbstractFont<TFont, TRenderer, TResource>(renderer, id, scale) {}
            ~AbstractBitmapFont() {}
        };

        template <typename TRenderContext, typename TRenderer, typename TTexture, typename TResource, typename TFont>
        class AbstractRenderContext : public IRenderable,
                                      public HasRenderer<TRenderer>,
                                      public AIDEntity<TRenderContext>,
                                      public HasAllocWrapper<TRenderContext, const TRenderer*, const std::string&>
        {
        public:
            AbstractRenderContext(const TRenderer* renderer, const std::string& id)
                : AIDEntity<TRenderContext>(id), HasRenderer<TRenderer>(renderer)
            {
                LogDebug("[{}] Created new RenderContext {}", this->renderer->getID(), this->getID());
            }
            ~AbstractRenderContext() { reinterpret_cast<TRenderContext*>(this)->Cleanup(); }
            void RenderRect(const Rect_t& rect, ColorU32_t color)
            {
                this->__prepare_rect_vertices(rect, color);
                this->__prepare_rect_index_chunk(IndexChunkType::BASIC);
            }
            void RenderTexturedRect(const Rect_t& rect, ColorU32_t color, const TTexture* texture)
            {
                IWA_ASSERT(this->renderer == texture->renderer);

                this->__prepare_rect_vertices(rect, color);
                this->__prepare_rect_index_chunk(IndexChunkType::TEXTURED, texture->getResource());
            }

            void RenderChar(Vec2_t pos, const TFont& font, uint8_t character, ColorU32_t color)
            {
                this->__render_char_impl(pos, font, character, color, font.getCharset(character));
            }

            void RenderWChar(Vec2_t pos, const TFont& font, char32_t character, ColorU32_t color)
            {
                this->__render_char_impl(pos, font, character, color, font.getCharset(character));
            }

            void RenderText(Vec2_t pos, const TFont& font, std::string_view text, ColorU32_t color)
            {
                for (char character : text)
                {
                    this->__render_char_impl(pos, font, character, color, font.getCharset(character));
                }
            }

            void RenderWText(Vec2_t pos, const TFont& font, std::wstring_view text, ColorU32_t color)
            {
                for (char32_t character : text)
                {
                    this->__render_char_impl(pos, font, character, color, font.getCharset(character));
                }
            }

        protected:
            std::vector<Vertex_t> __vertices = {};
            std::vector<IndexSize_t> __indicies = {};
            std::vector<IndexChunk_t> __indicies_chunks = {};

            inline void __render_char_impl(Vec2_t& pos, const TFont& font, char32_t character, ColorU32_t color, const Charset_t& charset)
            {
                stbtt_aligned_quad char_data{};

                stbtt_GetPackedQuad(charset.vec.data(), font.getAtlasSize().x, font.getAtlasSize().y,
                                    character - charset.first_char, &pos.x, &pos.y, &char_data, 0);

                Rect_t uv;
                uv.min = {char_data.s0, char_data.t0};
                uv.max = {char_data.s1, char_data.t1};

                Rect_t rect;
                rect.min = {char_data.x0, char_data.y0};
                rect.max = {char_data.x1, char_data.y1};

                this->__prepare_rect_vertices(rect, color, uv);
                this->__prepare_rect_index_chunk(IndexChunkType::TEXT, font.getResource());
            }

            inline void __prepare_rect_vertices(const Rect_t& rect, ColorU32_t color,
                                                const Rect_t& uv_box = {0, 0, 1, 1})
            {
                uint32_t start_index = this->__vertices.size();

                this->__vertices.emplace_back(Vertex_t{.pos = rect.getTL(), .color = color, .uv = uv_box.getTL()});
                this->__vertices.emplace_back(Vertex_t{.pos = rect.getTR(), .color = color, .uv = uv_box.getTR()});
                this->__vertices.emplace_back(Vertex_t{.pos = rect.getBR(), .color = color, .uv = uv_box.getBR()});
                this->__vertices.emplace_back(Vertex_t{.pos = rect.getBL(), .color = color, .uv = uv_box.getBL()});

                this->__indicies.emplace_back(start_index);
                this->__indicies.emplace_back(start_index + 1);
                this->__indicies.emplace_back(start_index + 2);
                this->__indicies.emplace_back(start_index + 2);
                this->__indicies.emplace_back(start_index + 3);
                this->__indicies.emplace_back(start_index);
            }

            inline void __prepare_rect_index_chunk(IndexChunkType type, TResource* resource = nullptr)
            {
                if (this->__indicies_chunks.size() > 0)
                {
                    auto& index_pair = this->__indicies_chunks[this->__indicies_chunks.size() - 1];
                    if (index_pair.shader_type != type || index_pair.data != resource)
                    {
                        this->__indicies_chunks.emplace_back(
                            IndexChunk_t{.index = index_pair.index + 6, .shader_type = type, .data = resource});
                    }
                    else
                    {
                        index_pair.index += 6;
                    }
                }
                else
                    this->__indicies_chunks.emplace_back(
                        IndexChunk_t{.index = 6, .shader_type = type, .data = resource});
            }
        };

        template <typename TRenderer, typename TBackendData, typename TRenderContext>
        class AbstractRenderer : public AIDEntity<TRenderer>, public HasAllocWrapper<TRenderer, const std::string&>
        {
        public:
            AbstractRenderer(const std::string& id) : AIDEntity<TRenderer>(id) {}
            // Override in derived classes
            static constexpr const char* getRendererID()
            {
                static_assert(TRenderer::getRendererID == AbstractRenderer<TRenderer, TBackendData, TRenderContext>::getRendererID(),
                              "You must override method \"static constexpr const char* getID()\"");
                return "unnamed-renderer";
            }

            using ContextsMap = std::unordered_map<std::string, TRenderContext*>;

            const ContextsMap& getContexts() { return this->__contexts; }
            const TBackendData& getBackendData() const { return this->__bd; };

            virtual void Cleanup() = 0;
            virtual void PreRender() = 0;
            virtual void Render() = 0;
            virtual void ResizeViewport(Vec2_t size) { this->__viewport_size = size; }

            void AddRenderContext(TRenderContext* ctx)
            {
                const std::string& id = ctx->getID();
                if (this->__contexts.contains(id))
                    LogWarn("Rewriting context with id \"{}\".", id);

                this->__contexts.emplace(id, ctx);
            }

            const Vec2_t& getViewportSize() const { return this->__viewport_size; }

        protected:
            Vec2_t __viewport_size;
            ContextsMap __contexts{};
            TBackendData __bd{};
            uint64_t __frame_count = 0;
        };
    }

} // namespace Iwa
