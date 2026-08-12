#pragma once

#include <vector>
#include "signal.hpp"
#include "utils.hpp"
#include "easings.hpp"
#include "macroses.hpp"
#include "attributes.hpp"


namespace iwa
{
    using binded_values_t = std::unordered_set<float*>;
    // Must take and return values in range from 0 to 1
    using easing_fn = float (*)(float);

    enum tween_type
    {
        none = 2,        // Tween will stop at the end
        repeatable = 4,  // Tween will automatically revert time direction
        restartable = 8, // Tween will automatically restart from the start point
    };

    /** Class for animating values.
     *  - Use method value() to get actual value
     *  - Call tween::tickall(dt) before render process
     */
    class tween;
    /**
     * Parameters of tween class.
     * - Stylable 
     */

    struct range_t
    {
        float min = 0.0f;
        float max = 1.0f;
    };

    class tween final
    {
    public:
        COMPONENT(float, time, 0.0f)
        COMPONENT(float, speed, 0.0f)
        COMPONENT(float, rev_speed, 0.0f)
        COMPONENT(tween_type, type, tween_type::none)
        TOGGEABLE_COMPONENT(enabled, true)
        TOGGEABLE_COMPONENT(backwards, false)
        TOGGEABLE_COMPONENT(ticking, true)
        COMPONENT(range_t, range, range_t({.min = 0.0f, .max = 1.0f}))
        COMPONENT(easing_fn, speed_fn, easings::linear)
        COMPONENT(easing_fn, rev_speed_fn, easings::linear)
        SIGNAL_COMPONENT(float, update, nullptr)
        SIGNAL_COMPONENT(EMPTY, start, nullptr)
        SIGNAL_COMPONENT(EMPTY, end, nullptr)
        
        struct builder : stylable<builder>
        {
        public:
            

            builder(tween* instance) : instance(instance) {}
            tween *build()
            {
                instance->build();
                return instance;
            }
            tween *instance = nullptr;

            #undef BUILDER_TYPE
            #define BUILDER_TYPE builder

            VALUE(float, time)
            VALUE(tween_type, type)
            TOGGEABLE_VALUE(enabled)
            TOGGEABLE_VALUE(backwards)
            TOGGEABLE_VALUE(ticking)
            VALUE(float, speed)
            VALUE(easing_fn, speed_fn)
            VALUE(float, rev_speed)
            VALUE(easing_fn, rev_speed_fn)
            SIGNAL_VALUE(EMPTY, start)
            SIGNAL_VALUE(EMPTY, end)
            SIGNAL_VALUE(float, update)
            VALUE(range_t, range)
            builder* bind(float& value)
            {
                instance->bind(value);
                return this;
            }

            builder* bind(std::initializer_list<float*>& values)
            {
                instance->bind(values);
                return this;
            }

            #undef BUILDER_TYPE 

        };
        friend builder;

        tween();
        ~tween();
        void build();
        void tick(float dt);
        float value();
        void bind(float& value);
        void bind(std::initializer_list<float*>& values);
        void unbind(float& value);
        void clear_binded();
        static void tickall(float dt);

        builder* construct();
        bool active();

    private:
        void __add_update_signal();
        void __check_binded_values();
        binded_values_t* __binded_values = nullptr;
        builder* __builder;
        bool __reached_end = false;
        bool __reached_start = false;
        bool __builded = false; 
        float __value;
    };
}