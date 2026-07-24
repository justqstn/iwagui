#pragma once

#include "event.hpp"
#include "widget.hpp"

namespace iwa
{
    struct focusable
    {
    public:
        virtual bool focused() = 0;
        iwa::event<> enter;   // @brief Called when mouse is in widget's rect
        iwa::event<> leave;    // @brief Called when mouse is not in widget's rect
    protected:
        void handle_focus();
        void clear_focus();
        bool __entered;
    };
    
    struct clickable
    {
    public:
        virtual bool pressed() = 0;
        iwa::event<> up;
        iwa::event<> down;
    protected:
        void handle_click(u_short zindex);
        bool __pressed;
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