#include "logger.hpp"
#include "canvas.hpp"

using namespace iwa;


/**
 * iwa::abstract_canvas definitions
 */

void abstract_canvas::set_pos(ImVec2 pos)
{
    if (this->pos != pos)
    {
        this->pos = pos; 
        __recomputing = true;
    }
}

void abstract_canvas::set_pos_px(ImVec2 pos_px)
{
    if (this->pos_px != pos_px)
    {
        this->pos_px = pos_px; 
        __recomputing = true;
    }
}

ImVec2 abstract_canvas::scaled_pos(ImVec2 pos)
{
    return pos * __bounds.Max + __bounds.Min * ImVec2(1 - pos.x, 1 - pos.y); 
}

ImVec2 abstract_canvas::scaled_size(ImVec2 size)
{
    return size * (__bounds.Max - __bounds.Min);
}


void abstract_canvas::set_bounds(const ImRect& rect)
{
    if (__bounds != rect)
    {
        __bounds = rect;
        __recomputing = true;
    }   
}

void abstract_canvas::set_anchor(const ImVec2 anchor)
{
    if (this->anchor != anchor)
    {
        this->anchor = anchor;
        __recomputing = true;
    }
}

void abstract_canvas::recompute()
{
    __recomputing = true;
}


/**
 * iwa::canvas implementations
 */ 

void canvas::set_size(ImVec2 size)
{
    if (this->size != size)
    {
        this->size = size;
        __recomputing = true;
    }
}

void canvas::set_size_px(ImVec2 size_px)
{
    if (this->size_px != size_px)
    {
        this->size_px = size_px;
        __recomputing = true;
    }
}

void window_canvas::set_padding(const ImRect& padding)
{
    if (this->padding != padding)
    {
        this->padding = padding;
        __recomputing_padding = true;
    }
}

void window_canvas::set_padding_px(const ImRect& padding_px)
{
    if (this->padding_px != padding_px)
    {
        this->padding_px = padding_px;
        __recomputing_padding = true;
    }
}


ImRect& window_canvas::compute_padding()
{
    bool is_recomputing = __recomputing;
    auto& rect = compute_rect();

    if (__recomputing_padding || is_recomputing)
    {
        __ret_padding.Min = rect.Min + rect.Min * padding.Min + padding_px.Min;
        __ret_padding.Max = rect.Max - rect.Min * padding.Max - padding_px.Max;

        __recomputing_padding = false;
    }

    return __ret_padding;
}

ImRect& canvas::compute_rect()
{
    if (__recomputing)
    {
        this->__recomputing_padding = true;
        auto computed_pos = scaled_pos(pos) + this->pos_px;
        auto computed_size = scaled_size(size) + this->size_px;

        __rect.Min = computed_pos - computed_size * anchor;
        __rect.Max = computed_pos + computed_size * ImVec2(1 - anchor.x, 1 - anchor.y);
        __recomputing = false;
    }
    
    if (__bounds.GetArea() == 0) LOGE("Bounds are null %s", id.data())

    return __rect;
}




/**
 * iwa::text_canvas implementations
 */

void text_canvas::set_size(float size)
{
    if (this->size.value != size)
    {
        this->size.value = size;
    }
}


ImRect& text_canvas::compute_rect()
{
    if (__recomputing)
    {
        auto computed_pos = scaled_pos(pos) + this->pos_px;
        __rect.Min = computed_pos - calculate_text_size() * anchor;
        __rect.Max = computed_pos + calculate_text_size() * ImVec2(1 - anchor.x, 1 - anchor.y);
        __recomputing = false;
    }

    if (__bounds.GetArea() == 0) LOGE("Bounds are null %s", id.data())

    return __rect;
}