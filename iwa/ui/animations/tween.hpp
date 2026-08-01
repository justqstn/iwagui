#pragma once

#include "event.hpp"
#include "utils.hpp"
#include "attributes.hpp"
#include <vector>

namespace iwa
{
    // Must take and return values in range from 0 to 1
    using easing_fn = float (*)(float);

    enum tween_type
    {
        none = 2,        // Tween will stop at the end
        repeatable = 4,  // Tween will automatically revert time direction
        restartable = 8, // Tween will automatically restart from the start point
    };

    /** Class for animating values.
     *  - Use field .value to get actual value
     *  - Call tween::tickall(dt) before render process
     */
    class tween;
    /**
     * Parameters of tween class.
     * - Stylable 
     */
    struct tween_params : stylable<tween_params, tween>
    {
        friend tween;
        struct speed_params
        {
            float value;            // Speed value (delta time * value)
            easing_fn fn = nullptr; // See file iwa/ui/easings.hpp for default easing functions
        };

    public:
        tween* instance;
        float time = 0.0f;                             // Start point of an animation. Capped in range 0-1
        tween_type tp = tween_type::none;              // Type of tween animation behaviour.
        speed_params forward_speed = {1.0f, nullptr};  // Tween uses this speed if animation time goes from start to end.
        speed_params backward_speed = {1.0f, nullptr}; // Tween uses this speed if animation time goes from end to start.
        iwa::event<iwa::tween *> *end;                 // Calls when time value reach 1.
        iwa::event<iwa::tween *> *start;               // Calls when time value reach 0.
        bool enabled = true;                           // If it's not enabled, it won't update even if tween::tick() is called.
        bool backwards = false;                        // Determines is animation directed back (time goes from 1 to 0) or forth (time goes from 0 to 1)
        bool ticking = true;                           // if true, will tick when tween::tickall() is called

        inline void enable() { this->enabled = true; }   // Binding for enabling tween.
        inline void disable() { this->enabled = false; } // Binding for disabling tween.
        inline void speed(float speed)                   // Binding for setting speed. @note Sets the value for both speeds.
        {
            this->backward_speed.value = speed;
            this->forward_speed.value = speed;
        }
        inline void easing(easing_fn fn) // Binding for setting easing function. @note Sets the value for both speeds.
        {
            this->backward_speed.fn = fn;
            this->forward_speed.fn = fn;
        }
    };

    class tween
    {
    public:
        tween_params *data;                            // Tween variables and configurates.

        tween(tween_params *data);
        ~tween();
        void reset();                               
        void finish();
        void start();
        void backward();
        void enable();
        void disable();
        void toggle();
        void tick(float dt);
        float value;
        static void tickall(float dt);

    private:
        bool __reached_end = false;
        bool __reached_start = false;
    };
}