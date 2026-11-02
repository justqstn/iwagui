#pragma once

#include "config.hpp"
#include <compare>
#include <cstdint>
#include <format>
#include <string_view>
#include <type_traits>

namespace Iwa
{
    namespace __private__
    {
        struct Pixel
        {
            float value = 0.0f;

            constexpr explicit Pixel() = default;
            constexpr explicit Pixel(float value) noexcept
                : value(value) {}

            constexpr operator float() const noexcept { return value; }
            constexpr Pixel operator+() const noexcept { return *this; }
            constexpr Pixel operator-() const noexcept { return Pixel{-value}; }
            constexpr Pixel operator+(Pixel rhs) const noexcept { return Pixel{value + rhs.value}; }
            constexpr Pixel operator-(Pixel rhs) const noexcept { return Pixel{value - rhs.value}; }
            constexpr Pixel operator*(Pixel value) const noexcept { return Pixel{value.value * value.value}; }
            constexpr Pixel operator/(Pixel value) const noexcept { return Pixel{value.value / value.value}; }
            constexpr Pixel& operator+=(Pixel rhs) noexcept { value += rhs.value; return *this; }
            constexpr Pixel& operator-=(Pixel rhs) noexcept { value -= rhs.value; return *this; }
            constexpr Pixel& operator*=(Pixel value) noexcept { value *= value; return *this; }
            constexpr Pixel& operator/=(Pixel value) noexcept { value /= value; return *this; }

            constexpr auto operator<=>(const Pixel& rhs) const noexcept = default;
        };
    } // namespace __private__
}

constexpr Iwa::__private__::Pixel operator*(float scalar, Iwa::__private__::Pixel p) noexcept
{
    return Iwa::__private__::Pixel{scalar * p.value};
}

constexpr Iwa::__private__::Pixel operator""_px(unsigned long long value) noexcept
{
    return Iwa::__private__::Pixel{static_cast<float>(value)};
}

constexpr Iwa::__private__::Pixel operator""_px(long double value) noexcept
{
    return Iwa::__private__::Pixel{static_cast<float>(value)};
}

#define DECLARE_VEC2(name, type)\
struct name                                                                                                             \
{                                                                                                                       \
    type x;                                                                                                             \
    type y;                                                                                                             \
    constexpr name() = default;                                                                                         \
    constexpr name(type x, type y)                                      : x(x), y(y) {}                                 \
    constexpr explicit name(type value)                                 : x(value), y(value) {}                         \
    template <typename Other>                                                                                           \
        requires (!std::is_same_v<Other, name>) && requires(Other o) { o.x; o.y; }                                      \
    constexpr operator Other() const noexcept                                                                           \
    {                                                                                                                   \
        using other_type = std::remove_cvref_t<decltype(std::declval<Other>().x)>;                                      \
        return Other(other_type(x), other_type(y));                                                                     \
    }                                                                                                                   \
    constexpr auto operator<=>(const name&) const = default;                                                            \
    constexpr name& operator+=(const name& vec) noexcept                { x += vec.x; y += vec.y; return *this; }       \
    constexpr name& operator-=(const name& vec) noexcept                { x -= vec.x; y -= vec.y; return *this; }       \
    constexpr name& operator*=(const name& vec) noexcept                { x *= vec.x; y *= vec.y; return *this; }       \
    constexpr name& operator/=(const name& vec) noexcept                { x /= vec.x; y /= vec.y; return *this; }       \
    constexpr name& operator*=(type value) noexcept                     { x *= value; y *= value; return *this; }       \
    constexpr name& operator/=(type value) noexcept                     { x /= value; y /= value; return *this; }       \
};                                                                                                                      \
constexpr name operator+(const name& lhs, const name& rhs)  noexcept    { return name(lhs.x + rhs.x, lhs.y + rhs.y); }  \
constexpr name operator-(const name& lhs, const name& rhs)  noexcept    { return name(lhs.x - rhs.x, lhs.y - rhs.y); }  \
constexpr name operator*(const name& lhs, const name& rhs)  noexcept    { return name(lhs.x * rhs.x, lhs.y * rhs.y); }  \
constexpr name operator/(const name& lhs, const name& rhs)  noexcept    { return name(lhs.x / rhs.x, lhs.y / rhs.y); }  \
constexpr name operator*(const name& lhs, type rhs) noexcept            { return name(lhs.x * rhs, lhs.y * rhs); }      \
constexpr name operator/(const name& lhs, type rhs) noexcept            { return name(lhs.x / rhs, lhs.y / rhs); }      \

#define DECLARE_RECT(name, vec_type, type)\
struct name                                                                                             \
{                                                                                                       \
    vec_type min;                                                                                       \
    vec_type max;                                                                                       \
    constexpr name() = default;                                                                         \
    constexpr name(vec_type min, vec_type max) : min(min), max(max) {}                                  \
    constexpr name(type x0, type y0, type x1, type y1) : min(x0,y0), max(x1,y1) {}                      \
    template <typename Other>                                                                           \
        requires (!std::is_same_v<Other, name>) && requires(Other o) { o.min; o.max; }                  \
    constexpr operator Other() const noexcept                                                           \
    {                                                                                                   \
        using other_vec = std::remove_cvref_t<decltype(std::declval<Other>().min)>;                     \
        return Other(static_cast<other_vec>(min), static_cast<other_vec>(max));                         \
    }                                                                                                   \
    constexpr auto operator<=>(const name&) const = default;                                            \
    /* Top-left */constexpr vec_type getTL() const { return min; }                                      \
    /* Top-right */constexpr vec_type getTR() const { return {max.x, min.y}; }                          \
    /* Bottom-left */constexpr vec_type getBL() const { return {min.x, max.y}; }                        \
    /* Bottom-right */constexpr vec_type getBR() const { return max; }                                  \
    constexpr vec_type getSize() const { return max-min; }                                              \
    constexpr float getArea() const { return (max.x-min.x) * (max.y - min.y); }                         \
    constexpr float getWidth() const { return max.x - min.x; }                                          \
    constexpr float getHeight() const { return max.y - min.y; }                                         \
    constexpr float getL() const { return min.x; }                                                      \
    constexpr float getR() const { return max.x; }                                                      \
    constexpr float getB() const { return max.y; }                                                      \
    constexpr float getT() const { return min.y; }                                                      \
};                                                                                                      \


#ifndef IWA_USING_MATH_NAMESPACE
namespace Iwa
{
#endif

    struct Color_t
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;

        constexpr Color_t() noexcept = default;

        constexpr Color_t(int r, int g, int b, int a = 255) noexcept
            : r(static_cast<uint8_t>(r))
            , g(static_cast<uint8_t>(g))
            , b(static_cast<uint8_t>(b))
            , a(static_cast<uint8_t>(a))
        {
        }

        constexpr Color_t(float r, float g, float b, float a = 1.0f) noexcept
            : r(static_cast<uint8_t>(r * 255.0f + 0.5f))
            , g(static_cast<uint8_t>(g * 255.0f + 0.5f))
            , b(static_cast<uint8_t>(b * 255.0f + 0.5f))
            , a(static_cast<uint8_t>(a * 255.0f + 0.5f))
        {
        }

        constexpr Color_t(std::uint32_t rgba) noexcept
            : r(static_cast<uint8_t>(rgba & 0xFFu))
            , g(static_cast<uint8_t>((rgba >> 8) & 0xFFu))
            , b(static_cast<uint8_t>((rgba >> 16) & 0xFFu))
            , a(static_cast<uint8_t>((rgba >> 24) & 0xFFu))
        {
        }

        constexpr Color_t(std::string_view hex) noexcept
            : Color_t()
        {
            if (!hex.empty() && hex.front() == '#')
                hex.remove_prefix(1);

            const auto nibble = [](char c) constexpr noexcept -> uint8_t
            {
                if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
                if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
                if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
                return 0;
            };
            const auto byte = [&](std::size_t i) constexpr noexcept -> uint8_t
            {
                return static_cast<uint8_t>((nibble(hex[i]) << 4) | nibble(hex[i + 1]));
            };

            if (hex.size() == 8)
            {
                a = byte(6);
                b = byte(4);
                g = byte(2);
                r = byte(0);
            }
            else if (hex.size() == 6)
            {
                r = byte(0);
                g = byte(2);
                b = byte(4);
                a = 255;
            }
            else if (hex.size() == 4)
            {
                a = static_cast<uint8_t>(nibble(hex[3]) * 0x11u);
                b = static_cast<uint8_t>(nibble(hex[2]) * 0x11u);
                g = static_cast<uint8_t>(nibble(hex[1]) * 0x11u);
                r = static_cast<uint8_t>(nibble(hex[0]) * 0x11u);
            }
            else if (hex.size() == 3)
            {
                r = static_cast<uint8_t>(nibble(hex[2]) * 0x11u);
                g = static_cast<uint8_t>(nibble(hex[1]) * 0x11u);
                b = static_cast<uint8_t>(nibble(hex[0]) * 0x11u);
                a = 255;
            }
        }

        constexpr Color_t(const char* hex) noexcept
            : Color_t(std::string_view{hex})
        {
        }

        constexpr operator std::uint32_t() const noexcept
        {
            return static_cast<std::uint32_t>(r)
                | (static_cast<std::uint32_t>(g) << 8)
                | (static_cast<std::uint32_t>(b) << 16)
                | (static_cast<std::uint32_t>(a) << 24);
        }

        constexpr auto operator<=>(const Color_t&) const noexcept = default;

        constexpr Color_t operator+() const noexcept { return *this; }
        constexpr Color_t operator-() const noexcept
        {
            return Color_t(static_cast<int>(-r), static_cast<int>(-g), static_cast<int>(-b), static_cast<int>(-a));
        }

        constexpr Color_t& operator+=(const Color_t& rhs) noexcept
        {
            r = static_cast<uint8_t>(r + rhs.r);
            g = static_cast<uint8_t>(g + rhs.g);
            b = static_cast<uint8_t>(b + rhs.b);
            a = static_cast<uint8_t>(a + rhs.a);
            return *this;
        }
        constexpr Color_t& operator-=(const Color_t& rhs) noexcept
        {
            r = static_cast<uint8_t>(r - rhs.r);
            g = static_cast<uint8_t>(g - rhs.g);
            b = static_cast<uint8_t>(b - rhs.b);
            a = static_cast<uint8_t>(a - rhs.a);
            return *this;
        }
        constexpr Color_t& operator*=(const Color_t& rhs) noexcept
        {
            r = static_cast<uint8_t>(r * rhs.r);
            g = static_cast<uint8_t>(g * rhs.g);
            b = static_cast<uint8_t>(b * rhs.b);
            a = static_cast<uint8_t>(a * rhs.a);
            return *this;
        }
        constexpr Color_t& operator/=(const Color_t& rhs) noexcept
        {
            r = static_cast<uint8_t>(r / rhs.r);
            g = static_cast<uint8_t>(g / rhs.g);
            b = static_cast<uint8_t>(b / rhs.b);
            a = static_cast<uint8_t>(a / rhs.a);
            return *this;
        }
        constexpr Color_t& operator*=(float value) noexcept
        {
            r = static_cast<uint8_t>(r * value + 0.5f);
            g = static_cast<uint8_t>(g * value + 0.5f);
            b = static_cast<uint8_t>(b * value + 0.5f);
            a = static_cast<uint8_t>(a * value + 0.5f);
            return *this;
        }
        constexpr Color_t& operator/=(float value) noexcept
        {
            r = static_cast<uint8_t>(r / value + 0.5f);
            g = static_cast<uint8_t>(g / value + 0.5f);
            b = static_cast<uint8_t>(b / value + 0.5f);
            a = static_cast<uint8_t>(a / value + 0.5f);
            return *this;
        }
    };

    constexpr Color_t operator+(const Color_t& lhs, const Color_t& rhs) noexcept
    {
        return Color_t(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a);
    }
    constexpr Color_t operator-(const Color_t& lhs, const Color_t& rhs) noexcept
    {
        return Color_t(lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b, lhs.a - rhs.a);
    }
    constexpr Color_t operator*(const Color_t& lhs, const Color_t& rhs) noexcept
    {
        return Color_t(lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b, lhs.a * rhs.a);
    }
    constexpr Color_t operator/(const Color_t& lhs, const Color_t& rhs) noexcept
    {
        return Color_t(lhs.r / rhs.r, lhs.g / rhs.g, lhs.b / rhs.b, lhs.a / rhs.a);
    }
    constexpr Color_t operator*(const Color_t& lhs, float rhs) noexcept
    {
        return Color_t(
            static_cast<int>(lhs.r * rhs + 0.5f),
            static_cast<int>(lhs.g * rhs + 0.5f),
            static_cast<int>(lhs.b * rhs + 0.5f),
            static_cast<int>(lhs.a * rhs + 0.5f));
    }
    constexpr Color_t operator*(float lhs, const Color_t& rhs) noexcept
    {
        return rhs * lhs;
    }
    constexpr Color_t operator/(const Color_t& lhs, float rhs) noexcept
    {
        return Color_t(
            static_cast<int>(lhs.r / rhs + 0.5f),
            static_cast<int>(lhs.g / rhs + 0.5f),
            static_cast<int>(lhs.b / rhs + 0.5f),
            static_cast<int>(lhs.a / rhs + 0.5f));
    }

    DECLARE_VEC2(Vec2_t, float)
    using Vec2f_t = Vec2_t;
    DECLARE_VEC2(Vec2i_t, int)
    DECLARE_VEC2(Vec2px_t, Iwa::__private__::Pixel)

    DECLARE_RECT(Rect_t, Vec2_t, float)
    DECLARE_RECT(Rectpx_t, Vec2px_t, Iwa::__private__::Pixel)

    using ColorU32_t = std::uint32_t;

#ifndef IWA_USING_MATH_NAMESPACE
}
#endif



#ifdef IWA_USING_MATH_NAMESPACE
#define IWA_MATH_TYPE(name) name
#else
#define IWA_MATH_TYPE(name) ::Iwa::name
#endif

#define IWA_DECLARE_VEC2_FORMATTER(name)                                                                                \
template <>                                                                                                             \
struct formatter<IWA_MATH_TYPE(name)>                                                                                   \
{                                                                                                                       \
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }                                             \
    template <typename FormatContext>                                                                                   \
    auto format(const IWA_MATH_TYPE(name)& v, FormatContext& ctx) const                                                 \
    {                                                                                                                   \
        return format_to(ctx.out(), "({}, {})", v.x, v.y);                                                              \
    }                                                                                                                   \
}

#define IWA_DECLARE_RECT_FORMATTER(name)                                                                                \
template <>                                                                                                             \
struct formatter<IWA_MATH_TYPE(name)>                                                                                   \
{                                                                                                                       \
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }                                             \
    template <typename FormatContext>                                                                                   \
    auto format(const IWA_MATH_TYPE(name)& r, FormatContext& ctx) const                                                 \
    {                                                                                                                   \
        return format_to(ctx.out(), "[{} -> {}]", r.min, r.max);                                                        \
    }                                                                                                                   \
}

namespace std
{
    template <>
    struct formatter<Iwa::__private__::Pixel>
    {
        constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(Iwa::__private__::Pixel p, FormatContext& ctx) const
        {
            return format_to(ctx.out(), "{}px", p.value);
        }
    };

    IWA_DECLARE_VEC2_FORMATTER(Vec2_t);
    IWA_DECLARE_VEC2_FORMATTER(Vec2i_t);
    IWA_DECLARE_VEC2_FORMATTER(Vec2px_t);
    IWA_DECLARE_RECT_FORMATTER(Rect_t);
    IWA_DECLARE_RECT_FORMATTER(Rectpx_t);
} // namespace std

#undef DECLARE_VEC2
#undef DECLARE_RECT
#undef IWA_DECLARE_VEC2_FORMATTER
#undef IWA_DECLARE_RECT_FORMATTER
#undef IWA_MATH_TYPE
