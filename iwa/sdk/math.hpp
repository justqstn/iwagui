#pragma once

#include "config.hpp"
#include <cstdint>
#include <compare>
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
    constexpr name(type value)                                          : x(value), y(value) {}                         \
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
    constexpr auto operator<=>(const name&) const = default;                                            \
    /* Top-left */constexpr vec_type getTL() const { return this->min; }                                \
    /* Top-right */constexpr vec_type getTR() const { return {this->max.x, this->min.y}; }              \
    /* Bottom-left */constexpr vec_type getBL() const { return {this->min.x, this->max.y}; }            \
    /* Bottom-right */constexpr vec_type getBR() const { return this->max; }                            \
    constexpr vec_type getSize() const { return this->max-this->min; }                                  \
    constexpr float getArea() const { return (this->max.x-this->min.x) * (this->max.y - this->min.y); } \
};                                                                                                      \


#ifndef IWA_USING_MATH_NAMESPACE
namespace Iwa
{
#endif
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

#undef DECLARE_VEC2
#undef DECLARE_RECT
