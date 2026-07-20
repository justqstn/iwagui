#include "tween.hpp"

#include <cmath>
#include <vector>
#include "logger.hpp"

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
        if (params.backwards) params.time = params.end;
        else params.time = params.start;
    }
    
    tweens.emplace_back(this);

    LOGD("New tween, total count: %i", tweens.size());
}
void iwa::tween::reset() { this->data.time = this->data.start; }

void iwa::tween::finish() { this->data.time = this->data.end; }

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

    bool reached_end = params.time == params.end;
    bool reached_start = params.time == params.start;

    if (reached_end || reached_start)
    {
        if (params.tp & tween_type::repeatable)
        {
            params.backwards = !params.backwards;
        }
        else if (params.tp & tween_type::restartable)
        {
            if (params.backwards)
                params.time = params.end;
            else
                params.time = params.start;
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