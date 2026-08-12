#pragma once

#include "typedefs.hpp"
#include "macroses.hpp"
#include "utils.hpp"

namespace iwa
{
    class abstract_canvas
    {
        COMPONENT(uint, zindex, 0)
        CONST_COMPONENT(std::string, id, "")
        RECOMPUTABLE_COMPONENT(rect_t, bounds, rect_t())
        RECOMPUTABLE_COMPONENT(vec2, pos, vec2())
        RECOMPUTABLE_COMPONENT(vec2, pos_px, vec2())
        RECOMPUTABLE_COMPONENT(vec2, anchor, vec2())

    public:
        virtual void recompute();
        virtual vec2 compute_pos();
        virtual const rect_t& compute_rect() = 0;
    protected:
        bool __recomputing = false;
        bool __recomputing_padding = false;
        rect_t __cached_rect = rect_t();
    };

    class frame_canvas : virtual public abstract_canvas
    { 
        RECOMPUTABLE_COMPONENT(vec2, size, vec2())
        RECOMPUTABLE_COMPONENT(vec2, size_px, vec2())
        
    public:
        vec2 compute_size();
        const rect_t& compute_rect() override;
    };

    struct window_canvas : public frame_canvas
    {
        RECOMPUTABLE_COMPONENT(rect_t, padding, rect_t())
    public:
        const rect_t& compute_padding();
    protected:
        rect_t __cached_padding = rect_t();
    };

    class text_canvas : virtual public abstract_canvas
    {
        RECOMPUTABLE_COMPONENT(float, size, 0.0f)
        RECOMPUTABLE_COMPONENT(float, size_px, 0.0f)
    public:
        const rect_t& compute_rect() override;
    };
}