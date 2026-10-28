#pragma once

#include "config.hpp"
#include <cstdint>
#include <compare>


#ifndef IWA_USING_MATH_NAMESPACE
namespace Iwa
{
#endif


    struct Vec2_t
    {
        float x = 0.0f;
        float y = 0.0f;

        constexpr Vec2_t() = default;
        constexpr Vec2_t(float x, float y)      : x(x), y(y) {}
        constexpr Vec2_t(float value)           : x(value), y(value) {}

        constexpr auto operator<=>(const Vec2_t&) const = default;

        constexpr Vec2_t& operator+=(const Vec2_t& vec)     { x += vec.x; y += vec.y; return *this; }
        constexpr Vec2_t& operator-=(const Vec2_t& vec)     { x -= vec.x; y -= vec.y; return *this; }
        constexpr Vec2_t& operator*=(const Vec2_t& vec)     { x *= vec.x; y *= vec.y; return *this; }
        constexpr Vec2_t& operator/=(const Vec2_t& vec)     { x /= vec.x; y /= vec.y; return *this; }
        constexpr Vec2_t& operator*=(float value)           { x *= value; y *= value; return *this; }
        constexpr Vec2_t& operator/=(float value)           { x /= value; y /= value; return *this; }
    };

    constexpr Vec2_t operator+(const Vec2_t& lhs, const Vec2_t& rhs)    { return Vec2_t(lhs.x + rhs.x, lhs.y + rhs.y); }
    constexpr Vec2_t operator-(const Vec2_t& lhs, const Vec2_t& rhs)    { return Vec2_t(lhs.x - rhs.x, lhs.y - rhs.y); }
    constexpr Vec2_t operator*(const Vec2_t& lhs, const Vec2_t& rhs)    { return Vec2_t(lhs.x * rhs.x, lhs.y * rhs.y); }
    constexpr Vec2_t operator/(const Vec2_t& lhs, const Vec2_t& rhs)    { return Vec2_t(lhs.x / rhs.x, lhs.y / rhs.y); }
    constexpr Vec2_t operator*(const Vec2_t& lhs, float rhs)            { return Vec2_t(lhs.x * rhs, lhs.y * rhs); }
    constexpr Vec2_t operator/(const Vec2_t& lhs, float rhs)            { return Vec2_t(lhs.x / rhs, lhs.y / rhs); }

    struct Rect_t
    {
        Vec2_t min;
        Vec2_t max;

        constexpr Rect_t() = default;
        constexpr Rect_t(Vec2_t min, Vec2_t max) : min(min), max(max) {}
        constexpr Rect_t(float x0, float y0, float x1, float y1) : min(x0,y0), max(x1,y1) {}
        constexpr auto operator<=>(const Rect_t&) const = default;
        // Top-left
        constexpr Vec2_t getTL() const { return this->min; }
        // Top-right
        constexpr Vec2_t getTR() const { return {this->max.x, this->min.y}; }
        // Bottom-left
        constexpr Vec2_t getBL() const { return {this->min.x, this->max.y}; }
        // Bottom-right
        constexpr Vec2_t getBR() const { return this->max; }

        constexpr Vec2_t getSize() const { return this->max-this->min; }
        constexpr float getArea() const { return (this->max.x-this->min.x) * (this->max.y - this->min.y); }
    };

    using ColorU32_t = std::uint32_t;

#ifndef IWA_USING_MATH_NAMESPACE
}
#endif
