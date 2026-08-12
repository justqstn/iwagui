#pragma once

#include "widget.hpp"
#include "logger.hpp"
#include "attributes.hpp"

namespace iwa
{
    template <class abstract_class, class widget_t>
    class abstract_frame : public abstract_class, public frame_canvas
    {
        COMPONENT(float, rounding, 0.0f)
        COMPONENT(ImDrawFlags, drawflags, ImDrawFlags_None)
    public:
        template<class builder_t>
        struct abstract_frame_builder : abstract_widget_builder<builder_t, widget_t>
        {
            #undef BUILDER_TYPE
            #define BUILDER_TYPE builder_t

            RECOMPUTABLE_VALUE(vec2, size)
            RECOMPUTABLE_VALUE(vec2, size_px)
            VALUE(float, rounding)
            VALUE(ImDrawFlags, drawflags)

            void validate() override
            {
                this->abstract_widget_builder<builder_t, widget_t>::validate();
                auto object = static_cast<abstract_frame<abstract_class, widget_t>*>(this->instance);
                assert((object->__size != vec2() || object->__size_px != vec2()) && "Size is null");
            }

            #undef BUILDER_TYPE
        };  

        abstract_frame() : abstract_class() {}
        void render(float dt) override
        {
            auto &rect = this->compute_rect();  
            ImGui::GetForegroundDrawList()->AddRectFilled(rect.Min, rect.Max, iwa::apply_alpha(this->__color, this->__ctx->get_alpha()), this->__rounding, this->__drawflags);
        }

    protected:
        frame_shadow* shadow = nullptr;
        bool __clipping = false;
    };

    template <class widget_t>
    class abstract_interactible_frame : public abstract_frame<abstract_interactible_widget<widget_t>, widget_t> {};

    template <class widget_t>
    class abstract_hoverable_frame : public abstract_interactible_frame<widget_t>, public hoverable
    {
    public:
        template<class builder_t>
        struct abstract_hoverable_frame_builder : abstract_interactible_frame<widget_t>::abstract_frame_builder<builder_t>
        {
            #undef BUILDER_TYPE
            #define BUILDER_TYPE builder_t

            TOGGEABLE_VALUE(hovering)
            SIGNAL_VALUE(EMPTY, enter)
            SIGNAL_VALUE(EMPTY, leave)
            
            #undef BUILDER_TYPE
        };
    };

    template <class widget_t>
    class abstract_clickable_frame : public abstract_hoverable_frame<widget_t>, public clickable
    {
    public:
        template<class builder_t>
        struct abstract_clickable_frame_builder : abstract_hoverable_frame<widget_t>::abstract_hoverable_frame_builder<builder_t> 
        {
            #undef BUILDER_TYPE
            #define BUILDER_TYPE builder_t

            TOGGEABLE_VALUE(clicking)
            SIGNAL_VALUE(EMPTY, click)
            SIGNAL_VALUE(EMPTY, release)
            
            #undef BUILDER_TYPE
        };

        void handle_interact() override
        {
            this->clickable::handle_interact();
            this->hoverable::handle_interact();
        }
    };

    template <class widget_t>
    class abstract_draggable_frame : public abstract_interactible_frame<widget_t>, public draggable
    {
    public:
        template<class builder_t>
        struct abstract_draggable_frame_builder : abstract_interactible_frame<widget_t>::abstract_frame_builder<builder_t>
        {
            #undef BUILDER_TYPE
            #define BUILDER_TYPE builder_t

            TOGGEABLE_VALUE(dragging)
            SIGNAL_VALUE(EMPTY, enter)
            SIGNAL_VALUE(EMPTY, leave)
            SIGNAL_VALUE(EMPTY, click)
            SIGNAL_VALUE(EMPTY, release)
            SIGNAL_VALUE(EMPTY, start_dragging)
            SIGNAL_VALUE(EMPTY, end_dragging)
            
            #undef BUILDER_TYPE
        };

        void handle_interact() override
        {
            this->clickable::handle_interact();
            this->draggable::handle_interact();
            this->hoverable::handle_interact();
        }
    };

    template <class widget_t>
    class abstract_bounded_draggable_frame : public abstract_interactible_frame<widget_t>, public bounded_draggable
    {
    public:
        template<class builder_t>
        struct abstract_bounded_draggable_frame_builder : abstract_interactible_frame<widget_t>::abstract_frame_builder<builder_t>
        {
            #undef BUILDER_TYPE
            #define BUILDER_TYPE builder_t

            VALUE_PTR(rect_t, drag_bounds)
            TOGGEABLE_VALUE(dragging)
            SIGNAL_VALUE(EMPTY, enter)
            SIGNAL_VALUE(EMPTY, leave)
            SIGNAL_VALUE(EMPTY, click)
            SIGNAL_VALUE(EMPTY, release)
            SIGNAL_VALUE(EMPTY, start_dragging)
            SIGNAL_VALUE(EMPTY, end_dragging)
            SIGNAL_VALUE(side_t, collide_with_side)
            
            #undef BUILDER_TYPE
        };

        void handle_interact() override
        {
            this->clickable::handle_interact();
            this->bounded_draggable::handle_interact();
            this->hoverable::handle_interact();
        }
    };

    class frame final: public abstract_frame<abstract_widget<frame>, frame> 
    {
        public: struct builder : abstract_frame_builder<builder> {};
    };

    class hoverable_frame final : public abstract_hoverable_frame<hoverable_frame>
    {
        public: struct builder : abstract_hoverable_frame_builder<builder> {};
    };
    
    class clickable_frame final : public abstract_clickable_frame<clickable_frame>
    {
        public: struct builder : abstract_clickable_frame_builder<builder> {};
    };

    class draggable_frame final : public abstract_draggable_frame<draggable_frame>
    {
        public: struct builder : abstract_draggable_frame_builder<builder> {};
    };

    class bounded_draggable_frame final : public abstract_bounded_draggable_frame<bounded_draggable_frame>
    {
        public: struct builder : abstract_bounded_draggable_frame_builder<builder> {};
    };
}