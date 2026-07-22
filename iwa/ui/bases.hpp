#pragma once

#include "imgui.h"
#include "utils.hpp"
#include "event.hpp"
#include "logger.hpp"
#include <vector>

#define ASPECT_RATIO ImVec2(16,10) 

namespace iwa
{
    template <class T>
    struct stylable
    {
    public:
        using styling_fn = void (*)(T&);
        std::vector<styling_fn> styles;
        void style(styling_fn fn)
        {
            this->styles.emplace_back(fn);
        }
        void apply_styles(T &object)
        {
            for (auto fn : this->styles)
            {
                fn(object);
            }
            this->styles.clear();
        }
    };

    struct canvas
    {
    public:
        ImVec2 pos = {100,100};
        ImVec2 anchor = {0,0};
        bool scaled_pos = false;
        void pos_scaling();
        void set_bounds(const ImRect& rect);
        void set_anchor(ImVec2 anchor);
        ImVec2 compute(float x, float y);
        ImVec2 compute(ImVec2 vec);
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
        ImVec2 size = {100,100};        
        void set_size(ImVec2 size);
        void set_size(float value);
        void set_size(float x, float y);
        void size_scaling();
        void scaling();
        ImRect& compute_rect(); // @todo Get rid of getters
        bool scaled_size = false;
    protected:
        ImVec2 __orig_size;
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

    class widget
    {
    public:
        struct params
        {
            friend widget;
            ImU32 color;
            iwa::event<float> pre; // @brief Called before rendering. @param float Delta time.
            iwa::event<float> post; // @brief Called after rendering. @param float Delta time.
        };
        virtual void render(float dt) = 0;
    protected:
        unsigned int id;
    };
}