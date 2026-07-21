#include "bases.hpp"
#include "utils.hpp"
#include "logger.hpp"
/*
    this->canvas->min = this->pos - this->size * ImVec2(1 - this->anchor.x, 1 - this->anchor.y);;
    this->canvas->max = this->pos + this->size * ImVec2(ImAbs(this->anchor.x - 1), ImAbs(this->anchor.y - 1));
*/

using namespace iwa;

void canvas::scaling()
{
    this->is_scaled_pos = true;
    this->is_scaled_size = true;
    this->__orig_pos = this->pos;
    this->__orig_size = this->size;
    this->__saved_originals = true;
}

void canvas::set_bounds(const iwa::ImRect& __rect)
{
    this->__bounds = __rect;
    this->__recomputing = true;
}

void canvas::set_size(ImVec2 size)
{
    if (this->is_scaled_pos)
    {   
        this->__orig_size = size;
        this->__recomputing = true;
    }
    else
    {
        this->size = size;
    }
}

void canvas::set_size(float value)
{
    if (this->is_scaled_pos)
    {   
        this->__orig_size = {value,value};
        this->__recomputing = true;
    }
    else
    {
        this->size = {value,value};
    }
}

void canvas::set_size(float x, float y)
{
    if (this->is_scaled_pos)
    {   
        this->__orig_size = {x,y};
        this->__recomputing = true;
    }
    else
    {
        this->size = {x,y};
    }
}

void canvas::set_pos(ImVec2 pos)
{
    if (this->is_scaled_pos)
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
    if (this->is_scaled_pos)
    {   
        this->__orig_pos = {x,y};
        this->__recomputing = true;
    }
    else
    {
        this->pos = {x,y};
    }
}

ImVec2 canvas::scale(float x, float y)
{
    return ImVec2(x,y) * this->__bounds.max + this->__bounds.min; 
}
ImVec2 canvas::scale(ImVec2 vec)
{
    return vec * this->__bounds.max + this->__bounds.min; 
}

iwa::ImRect& canvas::compute_rect()
{
    if (this->__recomputing)
    {
        if (!this->__saved_originals)
        {
            this->__orig_pos = this->pos;
            this->__orig_size = this->size;
            this->__saved_originals = true;
        }
        if (this->is_scaled_pos) this->pos = this->scale(this->__orig_pos);
        if (this->is_scaled_size) this->size = this->scale(this->__orig_size);
        this->__rect.min = this->pos - this->size * ImVec2(1 - this->anchor.x, 1 - this->anchor.y);;
        this->__rect.max = this->pos + this->size * ImVec2(ImAbs(this->anchor.x - 1), ImAbs(this->anchor.y - 1));
        this->__recomputing = false;
    }

    return this->__rect;
}