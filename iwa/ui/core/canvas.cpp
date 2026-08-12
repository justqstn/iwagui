#include "canvas.hpp"
#include "logger.hpp"
#include <stacktrace>


using namespace iwa;

void abstract_canvas::recompute()
{
    LOG("RECOMPUTING", logger::COLOR_DARK_CYAN, "%s", __id.data())
    this->__recomputing = true;
    this->__recomputing_padding = true;
}

vec2 abstract_canvas::compute_pos()
{
    return (__pos * __bounds.Max + __bounds.Min * ImVec2(1 - __pos.x, 1 - __pos.y)) + __pos_px;
}

vec2 frame_canvas::compute_size()
{
    return __size * (__bounds.Max - __bounds.Min) + __size_px; 
}

const rect_t& frame_canvas::compute_rect()
{
    if (this->__recomputing)
    {
        // LOGI("%s recomp", __id.data())
        this->__recomputing_padding = true;
        auto computed_pos = this->compute_pos();
        auto computed_size = this->compute_size();

        this->__cached_rect.Min = computed_pos - computed_size * __anchor;
        this->__cached_rect.Max = computed_pos + computed_size * ImVec2(1 - __anchor.x, 1 - __anchor.y);
        this->__recomputing = false;
    }
    
    if (this->__bounds.GetArea() == 0) LOGE("\"%s\": Bounds are null", this->__id.data())

    return this->__cached_rect;
}