#pragma once

#include <string>
#include <vector>

#include "imgui.h"
#include "bases.hpp"
#include "event.hpp"

namespace iwa
{
    namespace widgets
    {
        enum class corners_flags
        {
            none = 0,
            upper_left = 2,
            upper_right = 4,
            lower_right = 8,
            lower_left = 16
        };

        enum class begin_flags
        {
            none = 0,
            hoverable = 2,
            closable = 4,
            moveable = 8
        };

        struct abstract_window_params : widget::params, plane_canvas
        {
            corners_flags corners = corners_flags::none;
            float rounding = 0;
            bool clipping = true;
        };

        class window : public widget, public parent
        {
        public:
            struct params : abstract_window_params
            {
                friend window;
                
            };
            params data;
            window(const params& data);
            void render(float dt) override;
            canvas& get_canvas() override; 
        };

        class head_window : public widget, public parent
        {
        public:
            struct params : abstract_window_params
            {
                friend head_window;
                
                std::string header;
                iwa::scaled_float header_thickness;
                ImU32 header_color;
                ImU32 header_line_color;
                ImU32 outline_color;
                begin_flags flags = begin_flags::none;
                void scaling();
            };
            params data;
            head_window(const params& data);
            void render(float dt) override;
            canvas& get_canvas() override;
        };
    }
}