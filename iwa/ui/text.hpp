#pragma once

#include "utils.hpp"
#include "bases.hpp"
#include <string>

namespace iwa
{
    namespace widgets 
    {
        class text : public widget
        {
        public:
            struct shadow_params
            {
                ImU32 color = ImColor(0,0,0,255);
                long double angle; // in radians
                float thickness;
                float distance;
                bool is_glow;
            };

            struct params : widget::params, text_canvas, stylable<text>
            {
            public:
                friend text;
                std::string text;
                std::string_view font = "";
                ImDrawList* drawlist = nullptr;
                shadow_params shadow;
            };
            
            
            text(const params& data);
            void render(float dt) override;
            canvas& get_canvas() override;
            params data;
        protected:
            void draw(float dt) override;
        };
    }
}