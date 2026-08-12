#pragma once

#include <concepts>

#include "macroses.hpp"
#include "canvas.hpp"

namespace iwa
{
    template <class T>
    struct stylable
    {
    public:
        using style_fn = void (*)(T *);

        T *style(style_fn fn)
        {
            fn(reinterpret_cast<T *>(this));
            return reinterpret_cast<T *>(this);
        }
    };

    template<class instance_t>
    struct abstract_builder : stylable<abstract_builder<instance_t>>
    {
    public:   
        abstract_builder() : instance(new instance_t) { this->instance->__builder_ptr = this; }
        instance_t* build() { this->validate(); instance->build(); return instance; }
        instance_t *instance = nullptr;
        virtual void validate() = 0;
    };

    template<class instance_t>
    struct buildable
    {
        template<typename T>
        friend class abstract_builder;
    public:
        virtual ~buildable()
        {
            // delete reinterpret_cast<abstract_builder<instance_t>*>(__builder_ptr);
        }


        virtual bool builded()
        {
            return __builded;
        }

        virtual void build()    
        {
            if (__builded)
                return; 
            if (__builder_ptr == nullptr)
                LOG("MEM LEAK", logger::COLOR_LIGHT_MAGENTA, "Memory leak: __builder_ptr is not initialized")

            __builded = true;
        }

    protected:
        bool __builded = false;
        abstract_builder<instance_t>* __builder_ptr = nullptr;
    };

    struct renderable
    {
        virtual void push(uint parent_zindex) = 0;
        virtual void render(float dt) = 0;
    };

    struct initializable
    {
    protected:
        bool __initialized = false; 
    };

    struct interactible : virtual abstract_canvas
    {
        virtual ~interactible();
        virtual void handle_interact() = 0; 
    };

    struct hoverable : virtual interactible
    {
        SIGNAL_COMPONENT(EMPTY, enter, nullptr)
        SIGNAL_COMPONENT(EMPTY, leave, nullptr)
        TOGGEABLE_COMPONENT(hovering, true)

        virtual ~hoverable();
        virtual bool hovered();
        void handle_interact() override;
    protected:
        bool __enter_callback = false;
    };

    struct clickable : virtual interactible
    {
        friend class input_controller;
        SIGNAL_COMPONENT(EMPTY, click, nullptr)
        SIGNAL_COMPONENT(EMPTY, release, nullptr)
        TOGGEABLE_COMPONENT(clicking, true)


        virtual ~clickable();   
        virtual bool pressed();
        void handle_interact() override;
    protected:
        bool __click_state = false;
        bool __click_callback = false;  
    };

    struct draggable : hoverable, clickable
    {
        COMPONENT_PTR(abstract_canvas, drag_canvas, this)
        SIGNAL_COMPONENT(EMPTY, start_dragging, nullptr)
        SIGNAL_COMPONENT(EMPTY, end_dragging, nullptr)
        TOGGEABLE_COMPONENT(dragging, true)

        virtual ~draggable();   
        bool pressed() override;
        void handle_interact() override;
    protected:
        vec2 __click_pos = vec2();
        bool __click_captured = false;
        bool __drag_callback = false;   
        bool __drag_started = false;
    };

    enum class side_t : char
    {
        none = 0,
        left = 2,
        right = 4,
        upper = 8,
        bottom = 16
    };

    inline side_t operator|(side_t a, side_t b)
    {
        return static_cast<side_t>(static_cast<char>(a) | static_cast<char>(b));
    }

    inline side_t operator&(side_t a, side_t b)
    {
        return static_cast<side_t>(static_cast<char>(a) & static_cast<char>(b));
    }

    inline side_t &operator|=(side_t &a, side_t b)
    {
        return a = a | b;
    }

    inline bool operator!(side_t side)
    {
        return side == side_t::none;
    }

    struct bounded_draggable : draggable
    {
        COMPONENT_PTR(rect_t, drag_bounds, new rect_t())
        SIGNAL_COMPONENT(side_t, collide_with_side, nullptr)
        virtual ~bounded_draggable();   
        void handle_interact() override;
    };

    struct frame_shadow
    {
            // #undef STRUCT
            // #define STRUCT frame_shadow

            // PARAM(ImVec2, offset, ImVec2())
            // PARAM(uint, color, ImColor(0,0,0,192))
            // PARAM(float, distance, 2.5f)
            // PARAM(float, thickness, 33.0f)

            // #undef STRUCT    
    };


}