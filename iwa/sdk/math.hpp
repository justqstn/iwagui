#pragma once

#include <cstdint>
#include <compare>

namespace Iwa
{
    struct Vec2_t
    {
        float x = 0.0f;
        float y = 0.0f;

        constexpr Vec2_t() = default;
        constexpr Vec2_t(float x, float y)      : x(x), y(y) {}
        constexpr Vec2_t(float Value)           : x(Value), y(Value) {}

        constexpr auto operator<=>(const Vec2_t&) const = default;

        constexpr Vec2_t& operator+=(const Vec2_t& vec)     { x += vec.x; y += vec.y; return *this; }
        constexpr Vec2_t& operator-=(const Vec2_t& vec)     { x -= vec.x; y -= vec.y; return *this; }
        constexpr Vec2_t& operator*=(const Vec2_t& vec)     { x *= vec.x; y *= vec.y; return *this; }
        constexpr Vec2_t& operator/=(const Vec2_t& vec)     { x /= vec.x; y /= vec.y; return *this; }
        constexpr Vec2_t& operator*=(float Value)           { x *= Value; y *= Value; return *this; }
        constexpr Vec2_t& operator/=(float Value)           { x /= Value; y /= Value; return *this; }
    };

    constexpr Vec2_t operator+(const Vec2_t& lhs, const Vec2_t& rhs)    { return Vec2_t(lhs.x + rhs.x, lhs.y + rhs.y); }
    constexpr Vec2_t operator-(const Vec2_t& lhs, const Vec2_t& rhs)    { return Vec2_t(lhs.x - rhs.x, lhs.y - rhs.y); }
    constexpr Vec2_t operator*(const Vec2_t& lhs, const Vec2_t& rhs)    { return Vec2_t(lhs.x * rhs.x, lhs.y * rhs.y); }
    constexpr Vec2_t operator/(const Vec2_t& lhs, const Vec2_t& rhs)    { return Vec2_t(lhs.x / rhs.x, lhs.y / rhs.y); }
    constexpr Vec2_t operator*(const Vec2_t& lhs, float rhs)            { return Vec2_t(lhs.x * rhs, lhs.y * rhs); }
    constexpr Vec2_t operator/(const Vec2_t& lhs, float rhs)            { return Vec2_t(lhs.x / rhs, lhs.y / rhs); }

    using ColorU32_t = std::uint32_t;
}
