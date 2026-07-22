#include "bases.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "config.hpp"
/*
    this->canvas->min = this->pos - this->size * ImVec2(1 - this->anchor.x, 1 - this->anchor.y);;
    this->canvas->max = this->pos + this->size * ImVec2(ImAbs(this->anchor.x - 1), ImAbs(this->anchor.y - 1));
*/

using namespace iwa;

void canvas::pos_scaling()
{
    this->scaled_pos = true;
}

void canvas::set_bounds(const ImRect& __rect)
{
    this->__bounds = __rect;
    this->__recomputing = true;
}

void canvas::set_anchor(const ImVec2 anchor)
{
    this->anchor = anchor;
    this->__recomputing = true;
}

void plane_canvas::set_size(ImVec2 size)
{
    if (this->scaled_pos)
    {   
        this->__orig_size = size;
        this->__recomputing = true;
    }
    else
    {
        this->size = size;
    }
}

void plane_canvas::set_size(float value)
{
    if (this->scaled_pos)
    {   
        this->__orig_size = {value,value};
        this->__recomputing = true;
    }
    else
    {
        this->size = {value,value};
    }
}

void plane_canvas::set_size(float x, float y)
{
    if (this->scaled_pos)
    {   
        this->__orig_size = {x,y};
        this->__recomputing = true;
    }
    else
    {
        this->size = {x,y};
    }
}


void text_canvas::set_size(float value)
{
    this->size.set(value);
}

void text_canvas::size_scaling()
{
    this->size.scaling();
}

void text_canvas::scaling()
{
    this->scaled_pos = true;
    this->size.scaling();
}

void canvas::set_pos(ImVec2 pos)
{
    if (this->scaled_pos)
    {   
        this->__orig_pos = pos;
        this->__recomputing = true;
    }
    else
    {
        this->pos = pos;
    }
}

void canvas::set_pos(float x, float y)
{
    if (this->scaled_pos)
    {   
        this->__orig_pos = {x,y};
        this->__recomputing = true;
    }
    else
    {
        this->pos = {x,y};
    }
}

ImVec2 canvas::compute(float x, float y)
{
    return ImVec2(ImAbs(1 - x), ImAbs(1 - y)) * this->__bounds.Max + this->__bounds.Min; 
}
ImVec2 canvas::compute(ImVec2 vec)
{
    return vec * this->__bounds.Max + this->__bounds.Min * ImVec2(ImAbs(1 - vec.x), ImAbs(1 - vec.y)); 
}

ImRect& plane_canvas::compute_rect()
{
    if (this->__recomputing)
    {
        if (!this->__saved_originals)
        {
            this->__orig_pos = this->pos;
            this->__orig_size = this->size;
            this->__saved_originals = true;
        }
        if (this->scaled_pos) this->pos = this->compute(this->__orig_pos);
        if (this->scaled_size) this->size = this->compute(this->__orig_size);
        this->__rect.Min = this->pos - this->size * this->anchor;
        this->__rect.Max = this->pos + this->size * ImVec2(1 - this->anchor.x, 1 - this->anchor.y);
        this->__recomputing = false;
    }

    return this->__rect;
}

void plane_canvas::size_scaling()
{
    this->scaled_size = true;
}

void plane_canvas::scaling()
{
    this->scaled_size = true;
    this->scaled_pos = true;
}

ImRect& text_canvas::compute_rect(ImVec2 text_size)
{
    if (this->__recomputing)
    {
        if (!this->__saved_originals)
        {
            this->__orig_pos = this->pos;
            this->__saved_originals = true;
        }
        if (this->scaled_pos) this->pos = this->compute(this->__orig_pos);
        this->__rect.Min = this->pos - text_size * this->anchor;
        this->__rect.Max = this->pos + text_size * ImVec2(1 - this->anchor.x, 1 - this->anchor.y);
        this->__recomputing = false;
    }

    return this->__rect;
}