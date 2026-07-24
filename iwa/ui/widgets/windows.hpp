#pragma once

#include <string>
#include <vector>

#include "imgui.h"
#include "canvas.hpp"
#include "attributes.hpp"
#include "event.hpp"

namespace iwa
{
    struct abstract_window_params : widget::params, plane_canvas, focusable
    {
        corners_flags corners = corners_flags::none;
        float rounding = 0;
        bool clipping = true;

        bool focused() override;
    };
    

    class window : public parent_widget
    {
    public:
        struct params : abstract_window_params, stylable<params, window>
        {
            friend window;
        };
        params data;
        window(const params& data);
        void render() override;
        canvas& get_canvas() override; 
    protected:
        void draw(float dt) override;
    };


    class head_window : public parent_widget
    {
    public:
        struct params : abstract_window_params, stylable<params, head_window>
        {
            friend head_window;
            
            std::string header;
            iwa::scaled_float header_thickness;
            ImU32 header_color;
            ImU32 header_line_color;
            ImU32 outline_color;
            void scaling();
        };
        params data;
        head_window(const params& data);
        void render() override;
        canvas& get_canvas() override;
    protected:
        void draw(float dt) override;
    };
}