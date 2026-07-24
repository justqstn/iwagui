#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "utils.hpp"

namespace iwa
{
    struct canvas
    {
    public:
        ImVec2 pos = {100,100};
        ImVec2 anchor = {0,0};
        bool scaled_pos = false;

        void pos_scaling();
        void set_bounds(const ImRect& rect);
        void set_anchor(ImVec2 anchor);
        ImVec2 compute_pos(ImVec2 vec);
        ImVec2 compute_size(ImVec2 vec);
        void set_pos(ImVec2 pos);
        void set_pos(float x, float y);
        
    protected:
        ImVec2 __orig_pos;
        ImRect __rect;
        ImRect __bounds; 
        bool __saved_originals = false;
        bool __recomputing = false;
    };

    struct plane_canvas : canvas
    {
    public:
        ImRect padding;
        ImVec2 size = {100,100};
        bool scaled_size = false;
        bool scaled_padding = false;

        void set_size(ImVec2 size);
        void set_size(float value);
        void set_size(float x, float y);
        void set_padding(const ImRect& rect);
        void size_scaling();
        void padding_scaling();
        void scaling();
        ImRect& compute_rect();
        ImRect& compute_padding();
    protected:
        ImVec2 __orig_size;
        ImRect __orig_padding;
        bool __recomputing_padding;
        bool __saved_padding_original;
    };

    struct text_canvas : canvas
    {
    public:
        iwa::scaled_float size;

        void size_scaling();
        void scaling();
        void set_size(float value);
    protected:
        ImRect& compute_rect(ImVec2 text_size);
        float __orig_size;
    };
}