#include "tween.hpp"

#include <cmath>
#include <vector>
#include "logger.hpp"

using namespace iwa;

namespace
{
    std::vector<tween*> tweens;
    float __dontchange(float f)
    {
        return f;
    }
}



tween::tween(tween_params* data)
{
    this->data = data;
    this->data->instance = this;
    auto params = this->data;
    params->apply_styles(this);
    if (params->forward_speed.fn == nullptr) params->forward_speed.fn = (easing_fn)(::__dontchange);
    if (params->backward_speed.fn == nullptr) params->backward_speed.fn = (easing_fn)(::__dontchange);

    if (params->enabled)
    {
        if (params->backwards) params->time = 1.0f;
        else params->time = 0.0f;
    }
    
    tweens.emplace_back(this);

    LOGD("New tween, total count: %i", tweens.size());
}

tween::~tween()
{
    delete this->data->start;
    delete this->data->end;
    delete this->data;
}

void tween::reset() { this->data->time = 0.0f; }

void tween::finish() { this->data->time = 1.0f; }

void tween::backward() { this->data->backwards = !this->data->backwards; }

void tween::enable() { this->data->enabled = true; }
void tween::disable() { this->data->enabled = false; }
void tween::toggle() { this->data->enabled = !this->data->enabled; }


void tween::tick(float dt)
{
    auto params = this->data;
    if (!params->enabled) return;

    if (!params->backwards)
    {
        params->time = std::min(params->time + dt * params->forward_speed.value, 1.0f);
        this->value = params->forward_speed.fn(params->time);
    }
    else
    {
        params->time = std::max(params->time - dt * params->backward_speed.value, 0.0f);
        this->value = params->backward_speed.fn(params->time);
    }

    bool reached_end = params->time == 1.0f;
    bool reached_start = params->time == 0.0f;

    if (reached_end || reached_start)
    {
        if (params->tp & tween_type::repeatable)
        {
            params->backwards = !params->backwards;
        }
        else if (params->tp & tween_type::restartable)
        {
            if (params->backwards)
                params->time = 1.0f;
            else
                params->time = 0.0f;
        }

        if (reached_end && !this->__reached_end)
        {
            this->__reached_end = true;
            LOGV("Event tween::end");
            if (params->end) params->end->call(this);
        }

        if (reached_start && !this->__reached_start)
        {
            this->__reached_start = true;
            LOGV("Event tween::start");
            if (params->start) params->start->call(this);
        }
    }
    if (!reached_end)
        this->__reached_end = false;
    if (!reached_start)
        this->__reached_start = false;
}

void tween::tickall(float dt)
{
    for (auto an : tweens)
    {
        if (an->data->ticking) an->tick(dt);
    }
}   