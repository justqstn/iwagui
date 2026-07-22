#pragma once

#include "utils.hpp"
#include "bases.hpp"
#include <string>

namespace iwa
{
    namespace widgets 
    {
        class text : widget
        {
        public:
            struct shadow_params : text_canvas
            {
                float thickness;
                float angle;
            };
            struct params : widget::params, text_canvas
            {
            public:
                friend text;
                std::string text;
                ImFont* font = nullptr;
                ImDrawList* drawlist = nullptr;
                shadow_params shadow;
            };
            text(const params& data);
            void render(float dt);
            params data;
        };
    }
}