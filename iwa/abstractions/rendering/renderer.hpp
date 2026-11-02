#pragma once

#include "abstractions/rendering/fonts.hpp"
#include "abstractions/rendering/types.hpp"
#include "utils/logger.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <deque>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace Iwa
{
    namespace __private__
    {
        template <typename TRenderer, typename TBackendData, typename TTexture, typename TResource, typename TBitmapFont, typename TDynamicFont>
        class AbstractRenderer : public HasInstanceMap<TRenderer>, public HasAllocWrapper<TRenderer, const std::string&>
        {
        public:
            AbstractRenderer(const std::string& id) : HasInstanceMap<TRenderer>(id)
            {
                LogDebug("Created new {} Renderer \"{}\"", static_cast<TRenderer*>(this)->getRendererID(),
                         this->getID());
            }

            ~AbstractRenderer() { LogDebug("Deleting Renderer \"{}\"", this->getID()); }

            TBackendData backend_data{};

            // Override in derived classes
            static constexpr const char* getRendererID()
            {
                static_assert(TRenderer::getRendererID ==
                                  AbstractRenderer<TRenderer, TBackendData, TTexture, TResource, TBitmapFont, TDynamicFont>::getRendererID,
                              "You must override method \"static constexpr const char* getID()\"");
                return "unnamed-renderer";
            }

            virtual void PreRender() = 0;
            virtual void Render() = 0;
            virtual void ResizeViewport() = 0;

            using ScissorEntry_t = std::pair<std::pair<size_t, size_t>, Rect_t>;

            void PushScissorRect(const Rect_t& rect)
            {
                const size_t begin = this->__indicies_chunks.size();
                this->__scissor_queue.push_back({{begin, std::numeric_limits<size_t>::max()}, rect});
            }

            void PopScissorRect()
            {
                for (auto it = this->__scissor_queue.rbegin(); it != this->__scissor_queue.rend(); ++it)
                {
                    if (it->first.second == std::numeric_limits<size_t>::max())
                    {
                        it->first.second = this->__indicies_chunks.size();
                        break;
                    }
                }
                this->__cut_index_chunk = true;
            }

            void RenderFilledRect(const Rect_t& rect, Color_t color, float rounding = 0.0f,
                                  Corners corners = Corners::ALL)
            {
                this->__prepare_rect_vertices(rect, color, {0, 0, 1, 1}, rounding, 0.0f, 0.0f, {0.0f, 0.0f}, corners);
                this->__prepare_index_chunk(IndexChunkType::RECT | IndexChunkType::FILLED, 6);
            }

            void RenderRect(const Rect_t& rect, Color_t color, float thickness = 1.0f, float rounding = 0.0f,
                            Corners corners = Corners::ALL)
            {
                thickness = std::max(thickness, 0.0f);
                rounding = std::clamp(rounding, 0.0f, std::min(rect.getWidth(), rect.getHeight()) * 0.5f);

                constexpr float kAAPad = 1.5f;
                const float expand = thickness * 0.5f + kAAPad;
                const Rect_t mesh_rect{rect.min - Vec2_t{expand, expand}, rect.max + Vec2_t{expand, expand}};

                this->__prepare_rect_vertices(mesh_rect, color, {0, 0, 1, 1}, rounding, 0.0f, thickness,
                                              {rect.getWidth(), rect.getHeight()}, corners);
                this->__prepare_index_chunk(IndexChunkType::RECT, 6);
            }

            void RenderLine(Vec2_t p0, Vec2_t p1, Color_t color, float thickness = 1.0f)
            {
                const IndexSize_t index_count = this->__prepare_line_vertices(p0, p1, color, thickness);
                if (index_count == 0)
                    return;
                this->__prepare_index_chunk(IndexChunkType::CIRCLE, index_count);
            }

            void RenderShadowLine(Vec2_t p0, Vec2_t p1, Color_t color, float size, float thickness,
                                  Vec2_t offset = {0.0f, 0.0f}, float line_thickness = 1.0f)
            {
                size = std::max(size, 0.0f);
                thickness = std::clamp(thickness, 0.0f, size);
                line_thickness = std::max(line_thickness, 1e-3f);

                const IndexSize_t index_count = this->__prepare_line_shadow_vertices(
                    p0 + offset, p1 + offset, color, line_thickness, size, thickness);
                if (index_count == 0)
                    return;
                this->__prepare_index_chunk(IndexChunkType::LINE | IndexChunkType::SHADOW, index_count);
            }

            void RenderShadowRect(const Rect_t& rect, Color_t color, float size, float thickness,
                                  Vec2_t offset = {0.0f, 0.0f}, float rounding = 0.0f,
                                  Corners corners = Corners::ALL)
            {
                size = std::max(size, 0.0f);
                thickness = std::clamp(thickness, 0.0f, size);
                rounding = std::clamp(rounding, 0.0f, std::min(rect.getWidth(), rect.getHeight()) * 0.5f);

                constexpr float kAAPad = 1.5f;
                const float expand = size + kAAPad;

                const Rect_t shadow_rect{rect.min + offset - Vec2_t{expand, expand},
                                         rect.max + offset + Vec2_t{expand, expand}};

                this->__prepare_rect_vertices(shadow_rect, color, {0, 0, 1, 1}, rounding, size, thickness, {0.0f, 0.0f},
                                              corners);
                this->__prepare_index_chunk(IndexChunkType::RECT | IndexChunkType::SHADOW, 6);
            }

            void RenderInnerShadowRect(const Rect_t& rect, Color_t color, float size, float thickness,
                                       Vec2_t offset = {0.0f, 0.0f}, float rounding = 0.0f,
                                       Corners corners = Corners::ALL)
            {
                size = std::max(size, 0.0f);
                thickness = std::clamp(thickness, 0.0f, size);
                rounding = std::clamp(rounding, 0.0f, std::min(rect.getWidth(), rect.getHeight()) * 0.5f);

                this->__prepare_rect_vertices(rect, color, {0, 0, 1, 1}, rounding, size, thickness, offset, corners);
                this->__prepare_index_chunk(IndexChunkType::RECT | IndexChunkType::INNER_SHADOW, 6);
            }

            void RenderTexturedRect(const Rect_t& rect, Color_t color, const TTexture* texture)
            {
                IWA_ASSERT(this->renderer == texture->renderer);

                this->__prepare_rect_vertices(rect, color);
                this->__prepare_index_chunk(IndexChunkType::TEXTURED, 6, texture->getResource());
            }

            void RenderCircle(Vec2_t center, float radius, Color_t color, int segments = 32)
            {
                const IndexSize_t index_count = this->__prepare_circle_vertices(center, radius, color, segments);
                this->__prepare_index_chunk(IndexChunkType::CIRCLE, index_count);
            }

            void RenderHollowCircle(Vec2_t center, float radius, Color_t color, float thickness = 1.0f,
                                    int segments = 32)
            {
                const IndexSize_t index_count =
                    this->__prepare_hollow_circle_vertices(center, radius, color, thickness, segments);
                if (index_count == 0)
                    return;
                this->__prepare_index_chunk(IndexChunkType::CIRCLE, index_count);
            }

            void RenderTexturedCircle(Vec2_t center, float radius, Color_t color, const TTexture* texture,
                                      int segments = 32)
            {
                IWA_ASSERT(this->renderer == texture->renderer);
                const IndexSize_t index_count = this->__prepare_circle_vertices(center, radius, color, segments);
                this->__prepare_index_chunk(IndexChunkType::CIRCLE | IndexChunkType::TEXTURED, index_count,
                                            texture->getResource());
            }

            void RenderShadowCircle(Vec2_t center, float radius, Color_t color, float size, float thickness,
                                    Vec2_t offset = {0.0f, 0.0f}, int segments = 32)
            {
                size = std::max(size, 0.0f);
                thickness = std::clamp(thickness, 0.0f, size);
                radius = std::max(radius, 0.0f);

                constexpr float kAAPad = 1.5f;
                const float mesh_radius = radius + size + kAAPad;
                const IndexSize_t index_count = this->__prepare_circle_vertices(
                    center + offset, mesh_radius, color, segments, radius, size, thickness);
                this->__prepare_index_chunk(IndexChunkType::CIRCLE | IndexChunkType::SHADOW, index_count);
            }

            void RenderInnerShadowCircle(Vec2_t center, float radius, Color_t color, float size, float thickness,
                                         Vec2_t offset = {0.0f, 0.0f}, int segments = 32)
            {
                size = std::max(size, 0.0f);
                thickness = std::clamp(thickness, 0.0f, size);
                radius = std::max(radius, 0.0f);

                const IndexSize_t index_count = this->__prepare_circle_vertices(
                    center, radius, color, segments, radius, size, thickness, offset);
                this->__prepare_index_chunk(IndexChunkType::CIRCLE | IndexChunkType::INNER_SHADOW, index_count);
            }

            void RenderFilledTriangle(Vec2_t p0, Vec2_t p1, Vec2_t p2, Color_t color)
            {
                const IndexSize_t index_count = this->__prepare_triangle_vertices(p0, p1, p2, color);
                this->__prepare_index_chunk(IndexChunkType::CIRCLE, index_count);
            }

            void RenderHollowTriangle(Vec2_t p0, Vec2_t p1, Vec2_t p2, Color_t color, float thickness = 1.0f)
            {
                IndexSize_t index_count = 0;
                index_count += this->__prepare_line_vertices(p0, p1, color, thickness);
                index_count += this->__prepare_line_vertices(p1, p2, color, thickness);
                index_count += this->__prepare_line_vertices(p2, p0, color, thickness);
                if (index_count == 0)
                    return;
                this->__prepare_index_chunk(IndexChunkType::CIRCLE, index_count);
            }

            template <typename TFont>
            void RenderChar(Vec2_t pos, const TFont& font, uint8_t character, Color_t color)
            {
                this->__render_char_impl(pos, font, character, color);
            }

            template <typename TFont>
            void RenderWChar(Vec2_t pos, const TFont& font, char32_t character, Color_t color)
            {
                this->__render_char_impl(pos, font, character, color);
            }

            template <typename TFont>
            void RenderText(Vec2_t pos, const TFont& font, std::string_view text, Color_t color)
            {
                for (unsigned char character : text)
                {
                    this->__render_char_impl(pos, font, character, color);
                }
            }

            template <typename TFont>
            void RenderWText(Vec2_t pos, const TFont& font, std::wstring_view text, Color_t color)
            {
                for (char32_t character : text)
                {
                    this->__render_char_impl(pos, font, character, color);
                }
            }

            const double& getDeltaTimeRef() const { return this->__delta_time; }
            const double getDeltaTime() const { return this->__delta_time; }
            Vec2_t getViewportSize() const { return this->__viewport_rect.getSize(); }

        protected:
            template <typename TFont>
            static constexpr bool __is_dynamic_font_v =
                std::is_base_of_v<AbstractDynamicFont<std::remove_cvref_t<TFont>, TRenderer, TResource>,
                                  std::remove_cvref_t<TFont>>;

            template <typename TFont>
            static constexpr bool __is_bitmap_font_v =
                std::is_base_of_v<AbstractBitmapFont<std::remove_cvref_t<TFont>, TRenderer, TResource>,
                                  std::remove_cvref_t<TFont>>;

            std::vector<Vertex_t> __vertices = {};
            std::vector<IndexSize_t> __indicies = {};
            std::vector<IndexChunk_t> __indicies_chunks = {};
            std::deque<ScissorEntry_t> __scissor_queue = {};
            double __delta_time = 1.0f / 60.0f;
            Rect_t __viewport_rect{0,0,0,0};
            uint64_t __frame_count = 0;

            void __resize_viewport_impl(const Rect_t& rect)
            {
                this->__viewport_rect = rect;
            }

            void __pre_render_impl() { __begin_frame_timepoint = clock_t::now(); }

            void __post_render_impl()
            {
                __delta_time = std::chrono::duration<double>(__begin_frame_timepoint - __end_frame_timepoint).count();
                __end_frame_timepoint = __begin_frame_timepoint;
                this->__cut_index_chunk = false;
            }

            template <typename TFont>
            inline void __render_char_impl(Vec2_t& pos, const TFont& font, char32_t character, ColorU32_t color)
            {
                static_assert(__is_dynamic_font_v<TFont> || __is_bitmap_font_v<TFont>,
                              "TFont must derive from AbstractBitmapFont or AbstractDynamicFont");

                const Rect_t rect = font.getCharRect(pos, character);
                const Rect_t uv = font.getCharUV(character);
                pos.x += font.getCharAdvance(character);

                this->__prepare_rect_vertices(rect, color, uv);

                if constexpr (__is_dynamic_font_v<TFont>)
                {
                    this->__prepare_index_chunk(IndexChunkType::TEXT_SDF, 6, font.getResource());
                }
                else
                {
                    this->__prepare_index_chunk(IndexChunkType::TEXT, 6, font.getResource());
                }
            }

            inline void __prepare_rect_vertices(const Rect_t& rect, ColorU32_t color,
                                                const Rect_t& uv_box = {0, 0, 1, 1}, float rounding = 0.0f,
                                                float shadow_size = 0.0f, float shadow_thickness = 0.0f,
                                                Vec2_t shadow_offset = {0.0f, 0.0f},
                                                Corners corners = Corners::ALL)
            {
                uint32_t start_index = this->__vertices.size();
                const float rect_w = std::max(rect.getWidth(), 1e-6f);
                const float rect_h = std::max(rect.getHeight(), 1e-6f);
                rounding = std::clamp(rounding, 0.0f, std::min(rect_w, rect_h) * 0.5f);
                const float corner_flags = static_cast<float>(static_cast<uint8_t>(corners));

                const auto make_vertex = [&](Vec2_t pos, Vec2_t uv) {
                    return Vertex_t{.pos = pos,
                                    .color = color,
                                    .uv = uv,
                                    .rect_w = rect_w,
                                    .rect_h = rect_h,
                                    .rounding = rounding,
                                    .shadow_size = shadow_size,
                                    .shadow_thickness = shadow_thickness,
                                    .shadow_offset_x = shadow_offset.x,
                                    .shadow_offset_y = shadow_offset.y,
                                    .corner_flags = corner_flags};
                };

                this->__vertices.emplace_back(make_vertex(rect.getTL(), uv_box.getTL()));
                this->__vertices.emplace_back(make_vertex(rect.getTR(), uv_box.getTR()));
                this->__vertices.emplace_back(make_vertex(rect.getBR(), uv_box.getBR()));
                this->__vertices.emplace_back(make_vertex(rect.getBL(), uv_box.getBL()));

                this->__indicies.emplace_back(start_index);
                this->__indicies.emplace_back(start_index + 1);
                this->__indicies.emplace_back(start_index + 2);
                this->__indicies.emplace_back(start_index + 2);
                this->__indicies.emplace_back(start_index + 3);
                this->__indicies.emplace_back(start_index);
            }

            inline IndexSize_t __prepare_line_vertices(Vec2_t p0, Vec2_t p1, ColorU32_t color, float thickness)
            {
                thickness = std::max(thickness, 1e-3f);
                const Vec2_t delta = p1 - p0;
                const float len_sq = delta.x * delta.x + delta.y * delta.y;
                if (len_sq < 1e-12f)
                    return 0;

                const float inv_len = 1.0f / std::sqrt(len_sq);
                const Vec2_t dir{delta.x * inv_len, delta.y * inv_len};
                Vec2_t normal{-dir.y, dir.x};
                const Vec2_t offset = normal * (thickness * 0.5f);

                const uint32_t start_index = static_cast<uint32_t>(this->__vertices.size());
                const auto make_vertex = [&](Vec2_t pos) {
                    return Vertex_t{.pos = pos,
                                    .color = color,
                                    .uv = {0.0f, 0.0f},
                                    .rect_w = 0.0f,
                                    .rect_h = 0.0f,
                                    .rounding = 0.0f,
                                    .shadow_size = 0.0f,
                                    .shadow_thickness = 0.0f,
                                    .shadow_offset_x = 0.0f,
                                    .shadow_offset_y = 0.0f};
                };

                this->__vertices.emplace_back(make_vertex(p0 - offset));
                this->__vertices.emplace_back(make_vertex(p0 + offset));
                this->__vertices.emplace_back(make_vertex(p1 + offset));
                this->__vertices.emplace_back(make_vertex(p1 - offset));

                this->__indicies.emplace_back(start_index + 0);
                this->__indicies.emplace_back(start_index + 1);
                this->__indicies.emplace_back(start_index + 2);
                this->__indicies.emplace_back(start_index + 0);
                this->__indicies.emplace_back(start_index + 2);
                this->__indicies.emplace_back(start_index + 3);

                return 6;
            }

            inline IndexSize_t __prepare_line_shadow_vertices(Vec2_t p0, Vec2_t p1, ColorU32_t color,
                                                              float line_thickness, float shadow_size,
                                                              float shadow_thickness)
            {
                const Vec2_t delta = p1 - p0;
                const float len_sq = delta.x * delta.x + delta.y * delta.y;
                if (len_sq < 1e-12f)
                    return 0;

                constexpr float kAAPad = 1.5f;
                const float len = std::sqrt(len_sq);
                const float inv_len = 1.0f / len;
                const Vec2_t dir{delta.x * inv_len, delta.y * inv_len};
                const Vec2_t normal{-dir.y, dir.x};

                const float core_r = line_thickness * 0.5f;
                const float half_len = len * 0.5f;
                const float mesh_half_along = half_len + core_r + shadow_size + kAAPad;
                const float mesh_half_perp = core_r + shadow_size + kAAPad;
                const Vec2_t center = (p0 + p1) * 0.5f;
                const Vec2_t along = dir * mesh_half_along;
                const Vec2_t perp = normal * mesh_half_perp;

                const uint32_t start_index = static_cast<uint32_t>(this->__vertices.size());
                const auto make_vertex = [&](Vec2_t pos, Vec2_t uv) {
                    return Vertex_t{.pos = pos,
                                    .color = color,
                                    .uv = uv,
                                    .rect_w = core_r,
                                    .rect_h = mesh_half_perp,
                                    .rounding = half_len,
                                    .shadow_size = shadow_size,
                                    .shadow_thickness = shadow_thickness,
                                    .shadow_offset_x = mesh_half_along,
                                    .shadow_offset_y = 0.0f,
                                    .corner_flags = 0.0f};
                };

                this->__vertices.emplace_back(make_vertex(center - along - perp, {0.0f, 0.0f}));
                this->__vertices.emplace_back(make_vertex(center + along - perp, {1.0f, 0.0f}));
                this->__vertices.emplace_back(make_vertex(center + along + perp, {1.0f, 1.0f}));
                this->__vertices.emplace_back(make_vertex(center - along + perp, {0.0f, 1.0f}));

                this->__indicies.emplace_back(start_index);
                this->__indicies.emplace_back(start_index + 1);
                this->__indicies.emplace_back(start_index + 2);
                this->__indicies.emplace_back(start_index + 2);
                this->__indicies.emplace_back(start_index + 3);
                this->__indicies.emplace_back(start_index);

                return 6;
            }

            inline IndexSize_t __prepare_triangle_vertices(Vec2_t p0, Vec2_t p1, Vec2_t p2, ColorU32_t color)
            {
                const uint32_t start_index = static_cast<uint32_t>(this->__vertices.size());
                const auto make_vertex = [&](Vec2_t pos) {
                    return Vertex_t{.pos = pos,
                                    .color = color,
                                    .uv = {0.0f, 0.0f},
                                    .rect_w = 0.0f,
                                    .rect_h = 0.0f,
                                    .rounding = 0.0f,
                                    .shadow_size = 0.0f,
                                    .shadow_thickness = 0.0f,
                                    .shadow_offset_x = 0.0f,
                                    .shadow_offset_y = 0.0f};
                };

                this->__vertices.emplace_back(make_vertex(p0));
                this->__vertices.emplace_back(make_vertex(p1));
                this->__vertices.emplace_back(make_vertex(p2));
                this->__indicies.emplace_back(start_index);
                this->__indicies.emplace_back(start_index + 1);
                this->__indicies.emplace_back(start_index + 2);
                return 3;
            }

            inline IndexSize_t __prepare_circle_vertices(Vec2_t center, float mesh_radius, ColorU32_t color,
                                                         int segments, float shape_radius = -1.0f,
                                                         float shadow_size = 0.0f, float shadow_thickness = 0.0f,
                                                         Vec2_t shadow_offset = {0.0f, 0.0f})
            {
                segments = std::clamp(segments, 3, 256);
                mesh_radius = std::max(mesh_radius, 1e-6f);
                if (shape_radius < 0.0f)
                    shape_radius = mesh_radius;

                const uint32_t start_index = static_cast<uint32_t>(this->__vertices.size());
                const float inv_diameter = 0.5f / mesh_radius;

                const auto make_vertex = [&](Vec2_t pos) {
                    const Vec2_t uv{(pos.x - center.x) * inv_diameter + 0.5f,
                                    (pos.y - center.y) * inv_diameter + 0.5f};
                    return Vertex_t{.pos = pos,
                                    .color = color,
                                    .uv = uv,
                                    .rect_w = shape_radius,
                                    .rect_h = mesh_radius,
                                    .rounding = 0.0f,
                                    .shadow_size = shadow_size,
                                    .shadow_thickness = shadow_thickness,
                                    .shadow_offset_x = shadow_offset.x,
                                    .shadow_offset_y = shadow_offset.y};
                };

                this->__vertices.emplace_back(make_vertex(center));
                for (int i = 0; i < segments; ++i)
                {
                    const float angle =
                        (static_cast<float>(i) / static_cast<float>(segments)) * 2.0f * std::numbers::pi_v<float>;
                    this->__vertices.emplace_back(
                        make_vertex(center + Vec2_t{std::cos(angle), std::sin(angle)} * mesh_radius));
                }

                for (int i = 0; i < segments; ++i)
                {
                    const uint32_t i1 = start_index + 1 + static_cast<uint32_t>(i);
                    const uint32_t i2 = start_index + 1 + static_cast<uint32_t>((i + 1) % segments);
                    this->__indicies.emplace_back(start_index);
                    this->__indicies.emplace_back(i1);
                    this->__indicies.emplace_back(i2);
                }

                return static_cast<IndexSize_t>(segments * 3);
            }

            inline IndexSize_t __prepare_hollow_circle_vertices(Vec2_t center, float radius, ColorU32_t color,
                                                                float thickness, int segments)
            {
                segments = std::clamp(segments, 3, 256);
                radius = std::max(radius, 0.0f);
                thickness = std::max(thickness, 1e-3f);

                const float outer_r = radius + thickness * 0.5f;
                const float inner_r = std::max(radius - thickness * 0.5f, 0.0f);
                if (outer_r <= 1e-6f)
                    return 0;

                const uint32_t start_index = static_cast<uint32_t>(this->__vertices.size());
                const auto make_vertex = [&](Vec2_t pos) {
                    return Vertex_t{.pos = pos,
                                    .color = color,
                                    .uv = {0.0f, 0.0f},
                                    .rect_w = radius,
                                    .rect_h = outer_r,
                                    .rounding = 0.0f,
                                    .shadow_size = 0.0f,
                                    .shadow_thickness = 0.0f,
                                    .shadow_offset_x = 0.0f,
                                    .shadow_offset_y = 0.0f};
                };

                for (int i = 0; i < segments; ++i)
                {
                    const float angle =
                        (static_cast<float>(i) / static_cast<float>(segments)) * 2.0f * std::numbers::pi_v<float>;
                    const Vec2_t dir{std::cos(angle), std::sin(angle)};
                    this->__vertices.emplace_back(make_vertex(center + dir * outer_r));
                    this->__vertices.emplace_back(make_vertex(center + dir * inner_r));
                }

                for (int i = 0; i < segments; ++i)
                {
                    const uint32_t i0 = start_index + static_cast<uint32_t>(i * 2);
                    const uint32_t i1 = start_index + static_cast<uint32_t>(((i + 1) % segments) * 2);
                    const uint32_t i0i = i0 + 1;
                    const uint32_t i1i = i1 + 1;
                    this->__indicies.emplace_back(i0);
                    this->__indicies.emplace_back(i1);
                    this->__indicies.emplace_back(i1i);
                    this->__indicies.emplace_back(i1i);
                    this->__indicies.emplace_back(i0i);
                    this->__indicies.emplace_back(i0);
                }

                return static_cast<IndexSize_t>(segments * 6);
            }

            inline void __prepare_index_chunk(IndexChunkType type, IndexSize_t index_count,
                                              TResource* resource = nullptr)
            {
                if (this->__indicies_chunks.size() > 0)
                {
                    auto& index_pair = this->__indicies_chunks[this->__indicies_chunks.size() - 1];
                    if (this->__cut_index_chunk || index_pair.shader_type != type || index_pair.data != resource)
                    {
                        this->__indicies_chunks.emplace_back(IndexChunk_t{
                            .index = index_pair.index + index_count, .shader_type = type, .data = resource});
                    }
                    else
                    {
                        index_pair.index += index_count;
                    }
                }
                else
                {
                    this->__indicies_chunks.emplace_back(
                        IndexChunk_t{.index = index_count, .shader_type = type, .data = resource});
                }

                this->__cut_index_chunk = false;
            }

        private:
            using clock_t = std::chrono::high_resolution_clock;
            clock_t::time_point __begin_frame_timepoint = clock_t::now();
            clock_t::time_point __end_frame_timepoint = clock_t::now();
            bool __cut_index_chunk = false;
        };

    } // namespace __private__
} // namespace Iwa
