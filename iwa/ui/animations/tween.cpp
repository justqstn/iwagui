#include "tween.hpp"

#include <cmath>
#include <vector>
#include "logger.hpp"
#include "macroses.hpp"

using namespace iwa;

namespace
{
    std::vector<tween*> tweens;
    float __dontchange(float f)
    {
        return f;
    }
}



tween::tween()
{

}

void tween::build()
{
    if (__builded) return;
    delete __builder;
    __builded = true;

    if (__enabled)
    {
        if (__backwards)
            __time = 1.0f;
        else
            __time = 0.0f;
    }

    tweens.emplace_back(this);

    LOGD("New tween, total count: %i", tweens.size());
}

bool tween::active()
{
    return __builded && __enabled;
}

tween::builder* tween::construct()
{
    __builder = new tween::builder(this); return new tween::builder(this);
}

tween::~tween()
{
    delete __start_signal;
    delete __end_signal;
}

float tween::value()
{
    return __value;
}

void tween::__add_update_signal()
{
    __update_signal = new signal<float>;
    __update_signal->add([binded_values = __binded_values](float value){
        for (auto binded : *binded_values)
        {
            *binded = value;
        }
    });
}

void tween::__check_binded_values()
{
    if (!__binded_values)
    {
        __binded_values = new binded_values_t();
        __add_update_signal();
    }
}

void tween::bind(float &value)
{
    __check_binded_values();
    __binded_values->emplace(&value);
}

void tween::bind(std::initializer_list<float *>& values)
{
    __check_binded_values();
    // __binded_values->emplace(values);
}

void tween::unbind(float &value)
{
    __check_binded_values();
    __binded_values->erase(&value);
}

void tween::clear_binded()
{
    __check_binded_values();
    __binded_values->clear();
}

void tween::tick(float dt)
{
   
    if (!__enabled) return;

    if (!__backwards)
    {
        __time = std::min(__time + dt * __speed, 1.0f);
        __value = ImLerp(__range.min, __range.max, __speed_fn(__time));
    }
    else
    {
        __time = std::max(__time - dt * __rev_speed, 0.0f);
        __value = ImLerp(__range.min, __range.max, __rev_speed_fn(__time));
    }

    CALL_SIGNAL(__update_signal, __value)

    bool reached_end = __time == 1.0f;
    bool reached_start = __time == 0.0f;

    if (reached_end || reached_start)
    {
        if (__type & tween_type::repeatable)
        {
            __backwards = !__backwards;
        }
        else if (__type & tween_type::restartable)
        {
            if (__backwards)
                __time = 1.0f;
            else
                __time = 0.0f;
        }

        if (reached_end && !__reached_end)
        {
            __reached_end = true;
            LOGV("Event tween::end");
            CALL_SIGNAL(__end_signal) 
        }

        if (reached_start && !__reached_start)
        {
            __reached_start = true;
            LOGV("Event tween::start");

            CALL_SIGNAL(__start_signal) 
        }
    }
    if (!reached_end)
        __reached_end = false;
    if (!reached_start)
        __reached_start = false;
}

void tween::tickall(float dt)
{
    for (auto an : tweens)
        if (an->__ticking && an->active()) 
            an->tick(dt);
}   