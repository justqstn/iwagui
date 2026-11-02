#pragma once

#include "thirdparty/stb_rect_pack.h"
#include "thirdparty/stb_truetype.h"

#include "abstractions/abstractions.hpp"
#include "utils/math.hpp"
#include "utils/logger.hpp"

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace Iwa
{
    struct CharRange_t
    {
        int first_char;
        int char_count;

        static CharRange_t ASCII() { return {.first_char = 32, .char_count = 95}; }
        static CharRange_t Cyrillic() { return {.first_char = 0x0400, .char_count = 0x04FF - 0x0400 + 1}; }
    };

    enum class Corners : uint8_t
    {
        NONE          = 0,
        TOP_LEFT      = 1 << 0,
        TOP_RIGHT     = 1 << 1,
        BOTTOM_RIGHT  = 1 << 2,
        BOTTOM_LEFT   = 1 << 3,
        TOP           = TOP_LEFT | TOP_RIGHT,
        BOTTOM        = BOTTOM_LEFT | BOTTOM_RIGHT,
        LEFT          = TOP_LEFT | BOTTOM_LEFT,
        RIGHT         = TOP_RIGHT | BOTTOM_RIGHT,
        ALL           = TOP_LEFT | TOP_RIGHT | BOTTOM_RIGHT | BOTTOM_LEFT,
    };

    constexpr Corners operator|(Corners lhs, Corners rhs) noexcept
    {
        return static_cast<Corners>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
    }

    constexpr Corners operator&(Corners lhs, Corners rhs) noexcept
    {
        return static_cast<Corners>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    constexpr Corners operator^(Corners lhs, Corners rhs) noexcept
    {
        return static_cast<Corners>(static_cast<uint8_t>(lhs) ^ static_cast<uint8_t>(rhs));
    }

    constexpr Corners operator~(Corners value) noexcept
    {
        return static_cast<Corners>(static_cast<uint8_t>(~static_cast<uint8_t>(value)));
    }

    constexpr Corners& operator|=(Corners& lhs, Corners rhs) noexcept
    {
        return lhs = lhs | rhs;
    }

    constexpr Corners& operator&=(Corners& lhs, Corners rhs) noexcept
    {
        return lhs = lhs & rhs;
    }

    constexpr Corners& operator^=(Corners& lhs, Corners rhs) noexcept
    {
        return lhs = lhs ^ rhs;
    }

    constexpr bool has_flag(Corners value, Corners flag) noexcept
    {
        return (value & flag) == flag;
    }

    namespace __private__
    {
        struct Vertex_t
        {
            Vec2_t pos = {0, 0};
            ColorU32_t color = 0;
            Vec2_t uv = {0, 0};
            float rect_w = 0.0f;
            float rect_h = 0.0f;
            float rounding = 0.0f;
            float shadow_size = 0.0f;
            float shadow_thickness = 0.0f;
            float shadow_offset_x = 0.0f;
            float shadow_offset_y = 0.0f;
            float corner_flags = static_cast<float>(static_cast<uint8_t>(Corners::ALL));
        };

        enum class IndexChunkType : uint16_t
        {
            NONE         = 0,
            RECT         = 1 << 0,
            CIRCLE       = 1 << 1,
            TEXTURED     = 1 << 2,
            TEXT         = 1 << 3,
            TEXT_SDF     = 1 << 4,
            SHADOW       = 1 << 5,
            INNER_SHADOW = 1 << 6,
            FILLED       = 1 << 7,
            LINE         = 1 << 8,
        };

        constexpr IndexChunkType operator|(IndexChunkType lhs, IndexChunkType rhs) noexcept
        {
            return static_cast<IndexChunkType>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
        }

        constexpr IndexChunkType operator&(IndexChunkType lhs, IndexChunkType rhs) noexcept
        {
            return static_cast<IndexChunkType>(static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs));
        }

        constexpr IndexChunkType operator^(IndexChunkType lhs, IndexChunkType rhs) noexcept
        {
            return static_cast<IndexChunkType>(static_cast<uint16_t>(lhs) ^ static_cast<uint16_t>(rhs));
        }

        constexpr IndexChunkType operator~(IndexChunkType value) noexcept
        {
            return static_cast<IndexChunkType>(static_cast<uint16_t>(~static_cast<uint16_t>(value)));
        }

        constexpr IndexChunkType& operator|=(IndexChunkType& lhs, IndexChunkType rhs) noexcept
        {
            return lhs = lhs | rhs;
        }

        constexpr IndexChunkType& operator&=(IndexChunkType& lhs, IndexChunkType rhs) noexcept
        {
            return lhs = lhs & rhs;
        }

        constexpr IndexChunkType& operator^=(IndexChunkType& lhs, IndexChunkType rhs) noexcept
        {
            return lhs = lhs ^ rhs;
        }

        constexpr bool has_flag(IndexChunkType value, IndexChunkType flag) noexcept
        {
            return (value & flag) == flag;
        }

        struct SdfGlyph_t
        {
            float u0 = 0, v0 = 0, u1 = 0, v1 = 0;
            float xoff = 0, yoff = 0;
            float width = 0, height = 0;
            float xadvance = 0;
        };

        struct SdfCharset_t
        {
            std::vector<SdfGlyph_t> vec;
            int first_char = 0;
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
            IndexChunkType shader_type = IndexChunkType::RECT | IndexChunkType::FILLED;
            void* data = nullptr;
        };

        struct Charset_t
        {
            std::vector<stbtt_packedchar> vec;
            int first_char;
        };

        template <typename TTexture, typename TRenderer, typename TResource>
        class AbstractTexture : public HasInstanceMap<TTexture>,
                                public HasRenderer<TRenderer>,
                                public HasResource<TResource>,
                                public HasAllocWrapper<TTexture, const TRenderer*, const std::string>
        {
        public:
            virtual bool Load(const std::span<const uint8_t> data) = 0;
            virtual bool Load(const std::filesystem::path& path) = 0;

        protected:
            AbstractTexture(const TRenderer* renderer, const std::string& id)
                : __private__::HasInstanceMap<TTexture>(id), __private__::HasRenderer<TRenderer>(renderer)
            {
            }

            ~AbstractTexture() { LogDebug("Deleting Texture \"{}\"", this->getID()); }

            Vec2_t __size = {-1.0f, -1.0f};
        };
    } // namespace __private__
} // namespace Iwa

