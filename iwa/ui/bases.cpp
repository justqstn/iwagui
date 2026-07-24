#include "bases.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "config.hpp"

#include <vector>

using namespace iwa;

std::vector<widget*> widgets;

unsigned int widget::get_id()
{
    if (!this->__id_initialized)
    {
        LOGI("realitat %p", this);
        this->__id_initialized = true;
        widgets.emplace_back(this);
        this->__id = widgets.size();
    }
    return this->__id;
}

widget::widget()
{
    this->__id = this->get_id();
    
    LOGD("Widget with unique id %i added.", this->__id);
}

widget* widget::get(unsigned int id)
{
    return widgets[id-1];
}

void parent::add_widget(unsigned int id)
{
    this->__widgets.emplace_back(id);
}

void parent::add_widget(widget& object)
{
    this->__widgets.emplace_back(object.get_id());
    
}

void canvas::pos_scaling()
{
    this->scaled_pos = true;
    this->__recomputing = true;
}

void canvas::set_bounds(const ImRect& __rect)
{
    if (this->__bounds != __rect)
    {
        this->__bounds = __rect;
        this->__recomputing = true;
    }   
}

void canvas::set_anchor(const ImVec2 anchor)
{
    if (this->anchor != anchor)
    {
        this->anchor = anchor;
        this->__recomputing = true;
    }
}

void plane_canvas::set_size(ImVec2 size)
{
    if (this->scaled_pos && this->__orig_size != size)
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
    if (this->scaled_pos && this->__orig_size.x != value && this->__orig_size.y != value)
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
    if (this->scaled_pos && this->__orig_size.x != x && this->__orig_size.y != y)
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
    this->__recomputing = true;

}

void canvas::set_pos(ImVec2 pos)
{
    if (this->scaled_pos && this->__orig_pos != pos)
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
    if (this->scaled_pos && this->__orig_pos.x != x && this->__orig_pos.y != y)
    {   
        this->__orig_pos = {x,y};
        this->__recomputing = true;
    }
    else
    {
        this->pos = {x,y};
    }
}

ImVec2 canvas::compute_pos(ImVec2 vec)
{
    return vec * this->__bounds.Max + this->__bounds.Min * ImVec2(1 - vec.x, 1 - vec.y); 
}

ImVec2 canvas::compute_size(ImVec2 vec)
{
    return vec * (this->__bounds.Max - this->__bounds.Min);
}

ImRect& plane_canvas::compute_padding()
{
    if (!this->__saved_padding_original)
    {
        this->__saved_padding_original = true;
        this->__orig_padding = this->padding;
    }

    auto& rect = this->compute_rect();

    if (this->scaled_padding)
    {
        if (this->__recomputing_padding)
        {
            this->padding.Min = rect.Min + rect.Min * this->__orig_padding.Min;
            this->padding.Max = rect.Max - rect.Min * this->__orig_padding.Max;
            this->__recomputing_padding = false;
        }
    }
    else
    {
        this->padding.Min = rect.Min + this->__orig_padding.Min;
        this->padding.Max = rect.Max - this->__orig_padding.Max;
    }

    return this->padding;
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
        if (this->scaled_pos) this->pos = this->compute_pos(this->__orig_pos);
        if (this->scaled_size) this->size = this->compute_size(this->__orig_size);

        this->__rect.Min = this->pos - this->size * this->anchor;
        this->__rect.Max = this->pos + this->size * ImVec2(1 - this->anchor.x, 1 - this->anchor.y);
        this->__recomputing = false;
    }

    return this->__rect;
}

void plane_canvas::size_scaling()
{
    this->scaled_size = true;
    this->__recomputing = true;
}

void plane_canvas::padding_scaling()
{
    this->scaled_padding = true;
    this->__recomputing_padding = true;
}

void plane_canvas::scaling()
{
    this->padding_scaling();
    this->size_scaling();
    this->pos_scaling();
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
        if (this->scaled_pos) this->pos = this->compute_pos(this->__orig_pos);
        this->__rect.Min = this->pos - text_size * this->anchor;
        this->__rect.Max = this->pos + text_size * ImVec2(1 - this->anchor.x, 1 - this->anchor.y);
        this->__recomputing = false;
    }

    return this->__rect;
}