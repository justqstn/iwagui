#include "tween.hpp"

#include <cmath>
#include <vector>
#include "logger.hpp"

using namespace iwa::easings;

namespace
{
    std::vector<iwa::tween*> tweens;
    float __dontchange(float f)
    {
        return f;
    }
}

void iwa::tween::params::set_speed(float speed)
{
    this->backward_speed.value = speed;
    this->forward_speed.value = speed;
}


iwa::tween::tween(const params& data)
{
    this->data = data;
    auto& params = this->data;
    params.apply_styles(*this);
    if (params.forward_speed.fn == nullptr) params.forward_speed.fn = (iwa::easing_fn)(::__dontchange);
    if (params.backward_speed.fn == nullptr) params.backward_speed.fn = (iwa::easing_fn)(::__dontchange);

    if (params.enabled)
    {
        if (params.backwards) params.time = 1.0f;
        else params.time = 0.0f;
    }
    
    tweens.emplace_back(this);

    LOGD("New tween, total count: %i", tweens.size());
}
void iwa::tween::reset() { this->data.time = 0.0f; }

void iwa::tween::finish() { this->data.time = 1.0f; }

void iwa::tween::backward() { this->data.backwards = !this->data.backwards; }

void iwa::tween::enable() { this->data.enabled = true; }
void iwa::tween::disable() { this->data.enabled = false; }
void iwa::tween::toggle() { this->data.enabled = !this->data.enabled; }


void iwa::tween::tick(float dt)
{
    auto& params = this->data;
    if (!params.enabled) return;

    if (!params.backwards)
    {
        params.time = std::min(params.time + dt * params.forward_speed.value, 1.0f);
        this->value = params.forward_speed.fn(params.time);
    }
    else
    {
        params.time = std::max(params.time - dt * params.backward_speed.value, 0.0f);
        this->value = params.backward_speed.fn(params.time);
    }

    bool reached_end = params.time == 1.0f;
    bool reached_start = params.time == 0.0f;

    if (reached_end || reached_start)
    {
        if (params.tp & tween_type::repeatable)
        {
            params.backwards = !params.backwards;
        }
        else if (params.tp & tween_type::restartable)
        {
            if (params.backwards)
                params.time = 1.0f;
            else
                params.time = 0.0f;
        }

        if (reached_end && !this->__reached_end)
        {
            this->__reached_end = true;
            params.on_end.call(*this);
        }

        if (reached_start && !this->__reached_start)
        {
            this->__reached_start = true;
            params.on_start.call(*this);
        }
    }
    if (!reached_end)
        this->__reached_end = false;
    if (!reached_start)
        this->__reached_start = false;
}

void iwa::tween::tickall(float dt)
{
    for (auto an : tweens)
    {
        if (an->data.ticking) an->tick(dt);
    }
}

float iwa::easings::linear(float x) { return x; }

float in::sine(float x)         { return 1 - cosf((x * M_PI) / 2); }
float out::sine(float x)        { return sinf((x * M_PI) / 2); }
float inout::sine(float x)      { return -(cosf(M_PI * x) - 1) / 2; }

float in::cubic(float x)        { return x * x * x; }
float out::cubic(float x)       { return  1 - powf(1 - x, 3); }
float inout::cubic(float x)     { return x < 0.5 ? 4 * x * x * x : 1 - powf(-2 * x + 2, 3) / 2; }

float in::quint(float x)        {   return x * x * x * x * x; }
float out::quint(float x)       {   return  1 - powf(1 - x, 5); }
float inout::quint(float x)     {   return  x < 0.5 ? 16 * x * x * x * x * x : 1 - powf(-2 * x + 2, 5) / 2; }

float in::circ(float x)         { return 1 - sqrtf(1 - powf(x, 2));}
float out::circ(float x)        { return sqrtf(1 - powf(x - 1, 2)); }
float inout::circ(float x)      { return x < 0.5 ? (1 - sqrtf(1 - powf(2 * x, 2))) / 2 : (sqrtf(1 - powf(-2 * x + 2, 2)) + 1) / 2;}

float in::elastic(float x)      
{
    constexpr long double c4 = (2 * M_PI) / 3;

    return x == 0
        ? 0
        : x == 1
        ? 1
        : -powf(2, 10 * x - 10) * sinf((x * 10 - 10.75) * c4);
}
float out::elastic(float x)     
{
    constexpr long double c4 = (2 * M_PI) / 3;

    return x == 0
      ? 0
      : x == 1
      ? 1
      : powf(2, -10 * x) * sinf((x * 10 - 0.75) * c4) + 1;

}
float inout::elastic(float x)   
{
    constexpr long double c5 = (2 * M_PI) / 4.5;

    return x == 0
     ? 0
     : x == 1
     ? 1
     : x < 0.5
     ? -(powf(2, 20 * x - 10) * sinf((20 * x - 11.125) * c5)) / 2
     : (powf(2, -20 * x + 10) * sinf((20 * x - 11.125) * c5)) / 2 + 1;
}

float in::quad(float x)         { return x * x; }
float inout::quad(float x)      { return  1 - (1 - x) * (1 - x); }
float out::quad(float x)        { return  x < 0.5 ? 2 * x * x : 1 - powf(-2 * x + 2, 2) / 2; }

float in::quart(float x)        { return x * x * x * x; }
float out::quart(float x)       { return 1 - powf(1 - x, 4); }
float inout::quart(float x)     { return x < 0.5 ? 8 * x * x * x * x : 1 - powf(-2 * x + 2, 4) / 2; }

float in::expo(float x)         { return x == 0 ? 0 : powf(2, 10 * x - 10);}
float out::expo(float x)        { return x == 1 ? 1 : 1 - powf(2, -10 * x); }
float inout::expo(float x)      { return x == 0 ? 0 : x == 1 ? 1 : x < 0.5 ? powf(2, 20 * x - 10) / 2 : (2 - powf(2, -20 * x + 10)) / 2; }

float in::back(float x)
{
    constexpr long double c1 = 1.70158;
    constexpr long double c3 = c1 + 1;

    return c3 * x * x * x - c1 * x * x;
}
float out::back(float x)
{
    constexpr long double c1 = 1.70158;
    constexpr long double c3 = c1 + 1;

    return 1 + c3 * powf(x - 1, 3) + c1 * powf(x - 1, 2);
}
float inout::back(float x)
{
    constexpr long double c1 = 1.70158;
    constexpr long double c2 = c1 * 1.525;

    return x < 0.5
               ? (powf(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2
               : (powf(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
}

float in::bounce(float x) { return 1 - out::bounce(1 - x); }
float out::bounce(float x)
{
    constexpr long double n1 = 7.5625;
    constexpr long double d1 = 2.75;

    if (x < 1 / d1)
    {
        return n1 * x * x;
    }
    else if (x < 2 / d1)
    {
        return n1 * (x -= 1.5 / d1) * x + 0.75;
    }
    else if (x < 2.5 / d1)
    {
        return n1 * (x -= 2.25 / d1) * x + 0.9375;
    }
    else
    {
        return n1 * (x -= 2.625 / d1) * x + 0.984375;
    }
}
float inout::bounce(float x)
{
    return x < 0.5
               ? (1 - out::bounce(1 - 2 * x)) / 2
               : (1 + out::bounce(2 * x - 1)) / 2;
}