#pragma once

#include "imgui.h"
#include "utils.hpp"
#include "event.hpp"
#include "logger.hpp"
#include <vector>

#define ASPECT_RATIO ImVec2(16,10) 

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

    struct focusable
    {
    public:
        virtual bool focused() = 0;
        iwa::event<> enter;   // @brief Called when mouse is in widget's rect
        iwa::event<> leave;    // @brief Called when mouse is not in widget's rect
    protected:
        void handle_focus();
        void clear_focus();
        bool __is_entered = false;
    };

    class widget
    {
    public:
        friend class parent_widget;
        friend class zindex_manager;

        struct params
        {
        public:
            friend widget;
            ImU32 color;
            iwa::event<float> pre;  // @brief Called before rendering. @param float Delta time.
            iwa::event<float> post; // @brief Called after rendering. @param float Delta time.
            bool enabled = true;
            u_short zindex;
            void enable();
            void disable();
            void toggle();


        };
        widget();
        virtual void render(float dt) = 0;
        virtual canvas& get_canvas() = 0;
        static widget* get(unsigned int id);
        unsigned int get_id();
        
    protected:
        virtual void draw(float dt) = 0;

        bool __id_initialized;
        unsigned int __id;
        u_short __zindex;
        u_short __parent_zindex;
    };

    class parent_widget : public widget
    {
    public:
        void add_widget(widget& object);
        void add_widget(unsigned int id);
    protected:
        std::vector<unsigned int> __widgets;
    };

    template <class params_t, class instance_t>
    struct stylable
    {
    public:
        using style_fn = void (*)(params_t&);
        using style_post_fn = void(*)(instance_t&);
        
        void style(style_fn fn)
        {
            fn((params_t&)*this);
        }

        void style(std::initializer_list<style_fn> list)
        {
            for (auto fn : list)
            {
                fn((params_t&)*this);
            }
        }

        void style_post(style_post_fn fn)
        {
            this->__styles.emplace_back(fn);
        }

        void style_post(std::initializer_list<style_post_fn> list)
        {
            for (auto fn : list)
            {
                this->__styles.emplace_back(fn);
            }
        }

        void apply_styles(instance_t &object)
        {
            for (auto fn : this->__styles)
            {
                fn(object);
            }
            this->__styles.clear();
        }
    protected:
        std::vector<style_post_fn> __styles;
    };
}