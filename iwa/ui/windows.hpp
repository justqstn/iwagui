#pragma once

#include <string>

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

        class window : widget
        {
        public:
            struct params : plane_canvas, widget::params
            {
                friend window;
                corners_flags corners = corners_flags::none;
                float rounding = 0;
                // void render(float dt);
            };
            params data;
        };

        class begin : widget
        {
        public:
            struct params : window::params
            {
                friend begin;
                
                std::string header;
                iwa::scaled_float header_thickness;
                ImU32 header_color;
                ImU32 header_line_color;
                ImU32 outline_color;
                begin_flags flags = begin_flags::none;
                void scaling();
            };
            params data;
            begin(const params& data);
            void render(float dt);
        };
    }
}