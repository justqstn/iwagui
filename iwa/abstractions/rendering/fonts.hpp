#pragma once

#include "abstractions/rendering/types.hpp"
#include "utils/logger.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace Iwa
{
    namespace __private__
    {
        template <typename TFont, typename TRenderer, typename TResource>
        class AbstractFont : public HasInstanceMap<TFont>,
                             public HasRenderer<TRenderer>,
                             public HasResource<TResource>,
                             public HasAllocWrapper<TFont, const TRenderer*, const std::string&, float>
        {
        public:
            TFont* setAtlasSize(Vec2_t size)
            {
                this->__atlas_size = size;
                return static_cast<TFont*>(this);
            }
            Vec2_t getAtlasSize() const { return this->__atlas_size; }

            virtual TFont* LoadFromSystem(const std::filesystem::path& path,
                                          const std::span<const CharRange_t> ranges) = 0;
            virtual TFont* Load(const std::span<const uint8_t> data, const std::span<const CharRange_t> ranges) = 0;
            virtual TFont* Load(const std::filesystem::path& path, const std::span<const CharRange_t> ranges) = 0;
            virtual Rect_t getCharUV(char32_t codepoint) const = 0;
            virtual Rect_t getCharRect(Vec2_t pos, char32_t codepoint) const = 0;
            virtual float getCharAdvance(char32_t codepoint) const = 0;

            float getScale() const { return this->__scale; }

        protected:
            Vec2_t __atlas_size;
            float __scale;

            AbstractFont(const TRenderer* renderer, const std::string& id, float scale = 48.0f)
                : HasInstanceMap<TFont>(id), HasRenderer<TRenderer>(renderer), __scale(scale),
                  __atlas_size({1024, 1024})
            {
            }

            ~AbstractFont() { LogDebug("Deleting Font \"{}\"", this->getID()); }
        };

        template <typename TFont, typename TRenderer, typename TResource>
        class AbstractBitmapFont : public AbstractFont<TFont, TRenderer, TResource>
        {
        public:
            AbstractBitmapFont(const TRenderer* renderer, const std::string& id, float scale = 48.0f)
                : AbstractFont<TFont, TRenderer, TResource>(renderer, id, scale)
            {
            }

            const Charset_t* findCharset(char32_t codepoint) const
            {
                for (const auto& charset : this->__charsets)
                {
                    if (codepoint >= static_cast<char32_t>(charset.first_char) &&
                        codepoint < static_cast<char32_t>(charset.first_char + charset.vec.size()))
                    {
                        return &charset;
                    }
                }
                return nullptr;
            }

            const Charset_t& getCharset(char32_t codepoint) const
            {
                const Charset_t* charset = this->findCharset(codepoint);
                IWA_ASSERT(charset != nullptr && "Codepoint is not present in any charset");
                return *charset;
            }

            Rect_t getCharUV(char32_t codepoint) const override
            {
                const Charset_t* charset = this->findCharset(codepoint);
                if (charset == nullptr)
                    return {};

                float x = 0.0f, y = 0.0f;
                stbtt_aligned_quad quad{};
                stbtt_GetPackedQuad(charset->vec.data(), this->__atlas_size.x, this->__atlas_size.y,
                                    codepoint - charset->first_char, &x, &y, &quad, 0);
                return Rect_t{{quad.s0, quad.t0}, {quad.s1, quad.t1}};
            }

            Rect_t getCharRect(Vec2_t pos, char32_t codepoint) const override
            {
                const Charset_t* charset = this->findCharset(codepoint);
                if (charset == nullptr)
                    return {pos, pos};

                stbtt_aligned_quad quad{};
                stbtt_GetPackedQuad(charset->vec.data(), this->__atlas_size.x, this->__atlas_size.y,
                                    codepoint - charset->first_char, &pos.x, &pos.y, &quad, 0);
                return Rect_t{{quad.x0, quad.y0}, {quad.x1, quad.y1}};
            }

            float getCharAdvance(char32_t codepoint) const override
            {
                const Charset_t* charset = this->findCharset(codepoint);
                if (charset == nullptr)
                    return 0.0f;

                float x = 0.0f, y = 0.0f;
                stbtt_aligned_quad quad{};
                stbtt_GetPackedQuad(charset->vec.data(), this->__atlas_size.x, this->__atlas_size.y,
                                    codepoint - charset->first_char, &x, &y, &quad, 0);
                return x;
            }

        protected:
            std::vector<Charset_t> __charsets;

            // @warning: This method allocated vector, delete pointer after calling
            std::vector<uint8_t>* __alloc_and_pack_font_bitmap(const std::span<const uint8_t>& data,
                                                               const std::span<const CharRange_t> char_ranges)
            {
                auto pixels = new std::vector<uint8_t>(this->__atlas_size.x * this->__atlas_size.y);

                stbtt_pack_context pack_ctx{};

                if (!stbtt_PackBegin(&pack_ctx, pixels->data(), this->__atlas_size.x, this->__atlas_size.y, 0, 2,
                                     nullptr))
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
        };

        template <typename TFont, typename TRenderer, typename TResource>
        class AbstractDynamicFont : public AbstractFont<TFont, TRenderer, TResource>
        {
        public:
            AbstractDynamicFont(const TRenderer* renderer, const std::string& id, float bake_size = 32.0f)
                : AbstractFont<TFont, TRenderer, TResource>(renderer, id, bake_size), __runtime_size(bake_size)
            {
                this->__atlas_size = {2048.0f, 2048.0f};
            }

            TFont* setSize(float size)
            {
                this->__runtime_size = size;
                return static_cast<TFont*>(this);
            }

            float getSize() const { return this->__runtime_size; }
            float getBakeSize() const { return this->__scale; }
            float getSizeRatio() const { return this->__runtime_size / this->__scale; }

            unsigned char getOnEdgeValue() const { return this->__onedge_value; }

            const SdfCharset_t* findCharset(char32_t codepoint) const
            {
                for (const auto& charset : this->__charsets)
                {
                    if (codepoint >= static_cast<char32_t>(charset.first_char) &&
                        codepoint < static_cast<char32_t>(charset.first_char + charset.vec.size()))
                    {
                        return &charset;
                    }
                }
                return nullptr;
            }

            const SdfCharset_t& getCharset(char32_t codepoint) const
            {
                const SdfCharset_t* charset = this->findCharset(codepoint);
                IWA_ASSERT(charset != nullptr && "Codepoint is not present in any charset");
                return *charset;
            }

            Rect_t getCharUV(char32_t codepoint) const override
            {
                const SdfCharset_t* charset = this->findCharset(codepoint);
                if (charset == nullptr)
                    return {};

                const auto& glyph = charset->vec[codepoint - charset->first_char];
                return Rect_t{{glyph.u0, glyph.v0}, {glyph.u1, glyph.v1}};
            }

            Rect_t getCharRect(Vec2_t pos, char32_t codepoint) const override
            {
                const SdfCharset_t* charset = this->findCharset(codepoint);
                if (charset == nullptr)
                    return {pos, pos};

                const auto& glyph = charset->vec[codepoint - charset->first_char];
                const float scale = this->getSizeRatio();

                Rect_t rect;
                rect.min = {pos.x + glyph.xoff * scale, pos.y + glyph.yoff * scale};
                rect.max = {rect.min.x + glyph.width * scale, rect.min.y + glyph.height * scale};
                return rect;
            }

            float getCharAdvance(char32_t codepoint) const override
            {
                const SdfCharset_t* charset = this->findCharset(codepoint);
                if (charset == nullptr)
                    return 0.0f;

                const auto& glyph = charset->vec[codepoint - charset->first_char];
                return glyph.xadvance * this->getSizeRatio();
            }

        protected:
            std::vector<SdfCharset_t> __charsets;
            std::vector<uint8_t> __ttf_data;
            stbtt_fontinfo __font_info{};
            float __runtime_size = 32.0f;
            int __padding = 5;
            unsigned char __onedge_value = 180;

            // @warning: allocates vector, delete pointer after calling
            std::vector<uint8_t>* __alloc_and_pack_font_sdf(const std::span<const uint8_t>& data,
                                                            const std::span<const CharRange_t> char_ranges)
            {
                this->__ttf_data.assign(data.begin(), data.end());

                if (!stbtt_InitFont(&this->__font_info, this->__ttf_data.data(), 0))
                {
                    LogFatal("Couldn't load font: \"{}\" stbtt_InitFont() failed!", this->getID());
                }

                const float bake_scale = stbtt_ScaleForPixelHeight(&this->__font_info, this->__scale);
                const float pixel_dist_scale = static_cast<float>(this->__onedge_value) / static_cast<float>(this->__padding);

                int total_chars = 0;
                for (const auto& range : char_ranges)
                    total_chars += range.char_count;

                struct GlyphBitmap
                {
                    uint8_t* pixels = nullptr;
                    int width = 0;
                    int height = 0;
                    int xoff = 0;
                    int yoff = 0;
                    float xadvance = 0;
                    int codepoint = 0;
                };

                std::vector<GlyphBitmap> glyphs;
                glyphs.reserve(total_chars);

                for (const auto& range : char_ranges)
                {
                    for (int i = 0; i < range.char_count; ++i)
                    {
                        const int codepoint = range.first_char + i;
                        GlyphBitmap glyph{};
                        glyph.codepoint = codepoint;
                        glyph.pixels = stbtt_GetCodepointSDF(&this->__font_info, bake_scale, codepoint, this->__padding,
                                                             this->__onedge_value, pixel_dist_scale, &glyph.width,
                                                             &glyph.height, &glyph.xoff, &glyph.yoff);

                        int advance = 0, lsb = 0;
                        stbtt_GetCodepointHMetrics(&this->__font_info, codepoint, &advance, &lsb);
                        glyph.xadvance = advance * bake_scale;

                        if (glyph.pixels == nullptr)
                        {
                            glyph.width = this->__padding * 2;
                            glyph.height = this->__padding * 2;
                        }

                        glyphs.push_back(glyph);
                    }
                }

                std::vector<stbrp_rect> rects(glyphs.size());
                for (size_t i = 0; i < glyphs.size(); ++i)
                {
                    rects[i].id = static_cast<int>(i);
                    rects[i].w = static_cast<stbrp_coord>(glyphs[i].width);
                    rects[i].h = static_cast<stbrp_coord>(glyphs[i].height);
                    rects[i].x = 0;
                    rects[i].y = 0;
                    rects[i].was_packed = 0;
                }

                const int atlas_w = static_cast<int>(this->__atlas_size.x);
                const int atlas_h = static_cast<int>(this->__atlas_size.y);
                std::vector<stbrp_node> nodes(atlas_w);
                stbrp_context pack_ctx{};
                stbrp_init_target(&pack_ctx, atlas_w, atlas_h, nodes.data(), static_cast<int>(nodes.size()));

                if (!stbrp_pack_rects(&pack_ctx, rects.data(), static_cast<int>(rects.size())))
                {
                    LogError("SDF atlas pack did not fit all glyphs for font \"{}\"", this->getID());
                }

                auto* pixels = new std::vector<uint8_t>(static_cast<size_t>(atlas_w * atlas_h), 0);

                this->__charsets.clear();
                this->__charsets.reserve(char_ranges.size());

                size_t glyph_index = 0;
                for (const auto& range : char_ranges)
                {
                    SdfCharset_t charset{};
                    charset.first_char = range.first_char;
                    charset.vec.resize(range.char_count);

                    for (int i = 0; i < range.char_count; ++i, ++glyph_index)
                    {
                        auto& src = glyphs[glyph_index];
                        auto& dst = charset.vec[i];
                        const auto& rect = rects[glyph_index];

                        dst.xoff = static_cast<float>(src.xoff);
                        dst.yoff = static_cast<float>(src.yoff);
                        dst.width = static_cast<float>(src.width);
                        dst.height = static_cast<float>(src.height);
                        dst.xadvance = src.xadvance;

                        if (rect.was_packed && src.pixels != nullptr && src.width > 0 && src.height > 0)
                        {
                            for (int y = 0; y < src.height; ++y)
                            {
                                std::memcpy(pixels->data() + (rect.y + y) * atlas_w + rect.x,
                                            src.pixels + y * src.width, static_cast<size_t>(src.width));
                            }

                            dst.u0 = static_cast<float>(rect.x) / this->__atlas_size.x;
                            dst.v0 = static_cast<float>(rect.y) / this->__atlas_size.y;
                            dst.u1 = static_cast<float>(rect.x + src.width) / this->__atlas_size.x;
                            dst.v1 = static_cast<float>(rect.y + src.height) / this->__atlas_size.y;
                        }

                        if (src.pixels)
                            stbtt_FreeSDF(src.pixels, nullptr);
                    }

                    this->__charsets.push_back(std::move(charset));
                }

                std::sort(this->__charsets.begin(), this->__charsets.end(),
                          [](const SdfCharset_t& a, const SdfCharset_t& b) { return a.first_char < b.first_char; });

                return pixels;
            }
        };

    } // namespace __private__
} // namespace Iwa
