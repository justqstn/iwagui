#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "utils.hpp"
#include "event.hpp"

namespace iwa
{   
    struct abstract_canvas
    {
    public:
        std::string id;
        unsigned int zindex = 0;
        ImVec2 pos = {0.5,0.5};
        ImVec2 pos_px = {0,0};
        ImVec2 anchor = {0.5f,0.5f};

        virtual void set_bounds(const ImRect &rect) final;
        virtual void set_anchor(ImVec2 anchor) final;
        virtual ImVec2 scaled_pos(ImVec2 pos) final;
        virtual ImVec2 scaled_size(ImVec2 size) final;
        virtual void recompute() final;
        virtual void set_pos(ImVec2 pos) final;
        virtual void set_pos_px(ImVec2 pos_px) final;

        virtual ImRect& compute_rect() = 0;
    protected:
        ImRect __rect;
        ImRect __bounds; 
        bool __recomputing = true;
    };

    struct canvas : virtual abstract_canvas
    {
    public:
        ImVec2 size = {0.5,0.5};
        ImVec2 size_px = {0,0};

        void set_size(ImVec2 size);
        void set_size_px(ImVec2 size_px);
        
        ImRect& compute_rect() override;

    protected:
        bool __recomputing_padding = true;
    };

    struct window_canvas : canvas
    {
    public:
        ImRect padding;
        ImRect padding_px;

        void set_padding(const ImRect& padding);
        void set_padding_px(const ImRect& padding_px);



        ImRect& compute_padding();  
    protected:
        ImRect __ret_padding;
    };

    struct text_canvas : virtual abstract_canvas
    {
    public:
        iwa::scaled_float size;

        void set_size(float size);
        void set_size_px(float size_px);

        virtual ImVec2& calculate_text_size() = 0;
        ImRect& compute_rect() override;
    protected:
        ImVec2 __text_size;
    };
}