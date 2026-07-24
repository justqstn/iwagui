#pragma once

#include "event.hpp"
#include "utils.hpp"
#include "bases.hpp"
#include <vector>

namespace iwa
{
    using easing_fn = float(*)(float);

    namespace easings // Used easing formulas from https://easings.net. You can check how every of them looks.
    {
        float linear(float x);
        namespace out
        {
            float sine(float x);
            float cubic(float x);
            float quint(float x);
            float circ(float x);
            float elastic(float x);
            float quad(float x);
            float quart(float x);
            float expo(float x);
            float back(float x);
            float bounce(float x);
        }
        namespace in
        {
            float sine(float x);
            float cubic(float x);
            float quint(float x);
            float circ(float x);
            float elastic(float x);
            float quad(float x);
            float quart(float x);
            float expo(float x);
            float back(float x);
            float bounce(float x);
        }

        namespace inout
        {           
            float sine(float x);
            float cubic(float x);
            float quint(float x);
            float circ(float x);
            float elastic(float x);
            float quad(float x);
            float quart(float x);
            float expo(float x);
            float back(float x);
            float bounce(float x);
        }
    }

    enum tween_type
    {
        none = 2,
        repeatable = 4,
        restartable = 8,
    };

    class tween
    {
    public:
        struct params : stylable<tween>
        {
            friend tween;
            struct speed_params
            {
                float value;
                easing_fn fn = nullptr;
            };
        public:
            float time = 0.0f;
            speed_params forward_speed = {1.0f, nullptr};
            speed_params backward_speed = {1.0f, nullptr}; 
            tween_type tp = tween_type::none;
            iwa::event<iwa::tween&> on_end;
            iwa::event<iwa::tween&> on_start;
            bool enabled = true;
            bool backwards = false;
            bool ticking = true; // if true, will tick when .tickall() is called
            void set_speed(float speed);
        protected:
            int real = 1488;
        };

        tween(const tween::params& data);
        void reset();
        void finish();
        void backward();
        void enable();
        void disable();
        void toggle();
        void tick(float dt);
        float value;
        params data;
        static void tickall(float dt);
    private:
        bool __reached_end = false; 
        bool __reached_start = false; 
    };
}   