#pragma once

#include <string>
#include <vector>

#include "imgui.h"
#include "canvas.hpp"
#include "attributes.hpp"
#include "event.hpp"
#include "text.hpp"

namespace iwa
{
    struct abstract_window_params : abstract_widget_params, plane_canvas, focusable
    {
        float rounding = 0;
        ImDrawFlags drawflags = ImDrawFlags_None;
        bool clipping = true;
    };
    
    class window;
    struct window_params : abstract_window_params, stylable<window_params, window>
    {
        friend window;
    };
    class window : public abstract_widget<window_params>
    {
    public:
        window(window_params* data);
        void render(unsigned int parent_zindex) override;
        void draw(float dt) override;
    protected:
        void render_descendants(unsigned int parent_zindex) override;
        void push_to_depth_map(unsigned int parent_zindex) override;
    };


    class head_window;
    struct head_window_params : abstract_window_params, stylable<head_window_params, head_window>, draggable
    {
        friend head_window;

        iwa::text* header_text = nullptr;
        iwa::window* header_window = nullptr;
        float header_line_thickness;
        ImU32 header_line_color;
        ImU32 outline_color;
    };
    class head_window : public abstract_widget<head_window_params>
    {
    public:
        head_window(head_window_params* data);
        ~head_window();
        void render(unsigned int parent_zindex) override;
        void draw(float dt) override;
    protected:
        void render_descendants(unsigned int parent_zindex) override;
        void push_to_depth_map(unsigned int parent_zindex) override;
    };
}