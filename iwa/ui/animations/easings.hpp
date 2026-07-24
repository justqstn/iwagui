#pragma once
#include <cmath>

namespace iwa
{
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
}