#pragma once

#include "event.hpp"
#include "widget.hpp"

namespace iwa
{
    struct focusable : virtual abstract_canvas
    {
    public:
        virtual ~focusable();
        virtual bool focused();
        iwa::event<>* enter = nullptr;   // Called when mouse is in widget's rect bounds
        iwa::event<>* leave = nullptr;    // Called when mouse is not in widget's rect bounds
        void on_leave(iwa::event<>::callback_t fn);
        void on_enter(iwa::event<>::callback_t fn);
        bool focusable = true;
    protected:
        bool __entered = false;

        virtual void handle_focus();
        virtual void clear_focus();
    };
    
    struct clickable : virtual abstract_canvas
    {
    public:
        friend class depth_mapper;
        
        virtual ~clickable();
        iwa::event<>* up = nullptr;
        iwa::event<>* down = nullptr;
        void on_released(iwa::event<>::callback_t fn);
        void on_pressed(iwa::event<>::callback_t fn);
        bool clickable = true;

        virtual bool pressed();
    protected:
        bool __down_called = false;

        virtual void handle_click(unsigned int parent_zindex);
    };

    struct draggable : virtual clickable
    {
    public:
        virtual ~draggable();
        abstract_canvas* drag_canvas = this;
        iwa::event<>* start_drag = nullptr;
        iwa::event<>* end_drag = nullptr;
        void on_started_drag(iwa::event<>::callback_t fn);
        void on_ended_drag(iwa::event<>::callback_t fn);
        bool draggable = false;

        bool pressed() override;
    protected:
        ImVec2 __click_pos = {0,0};
        bool __click_captured = false;      
        bool __start_called = false;

        virtual void handle_dragging();
    };

    struct rect_draggable : draggable
    {
    protected:
        void handle_dragging();
    };

    template <class params_t, class instance_t>
    struct stylable
    {
    public:
        using style_fn = void (*)(params_t*);
        using style_post_fn = void(*)(instance_t*);
        
        void style(style_fn fn)
        {
            fn((params_t*)this);
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

        void apply_styles(void *widget)
        {
            for (auto fn : this->__styles)
            {
                fn((instance_t*)widget);
            }
            LOGV("Applied %i styles", this->__styles.size());
            this->__styles.clear();
        }
    protected:
        std::vector<style_post_fn> __styles;
    };
}