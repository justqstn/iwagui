#include "depth_mapper.hpp"

#ifdef DEPTH_MAPPER_DEBUG
#include <format>
#endif

using namespace iwa;

depth_mapper::depth_mapper()
{
#ifdef DEPTH_MAPPER_DEBUG
    this->__debug = "None";
#endif
}

void depth_mapper::push(base_widget* widget, unsigned int parent_zindex)
{
    if (!widget->alive()) return;
    auto zindex = widget->data()->zindex + parent_zindex;

    if (!this->__mapped_widgets.contains(zindex))
    {
        LOGV("Creating new ZIndex(%i) vector.", zindex);
        this->__mapped_widgets[zindex] = {};
    }

    LOGV("Object address: %p", widget);
    this->__mapped_widgets[zindex].emplace_back(widget);
}

void depth_mapper::input()
{
    LOGV("Input processing");
    if (this->__clickable != nullptr)
    {
        if (!this->__clickable->__down_called)
        {
            if (this->__clickable->down) this->__clickable->down->call();
            this->__clickable->__down_called = true;
        }
    }
}

void depth_mapper::render(float dt)
{
    LOGV("Starting drawing");

    #ifdef DEPTH_MAPPER_DEBUG 
    size_t drawn_widgets = 0; 
    #endif

    for (auto zindex : this->__mapped_widgets)
    {
        LOGV("ZIndex %i", zindex.first);

        #ifdef DEPTH_MAPPER_DEBUG 
        drawn_widgets += zindex.second.size(); 
        #endif

        for (auto widget : zindex.second)
        {
            if (widget->data()->pre) widget->data()->pre->call(dt);
            widget->draw(dt);
            if (widget->data()->post) widget->data()->post->call(dt);
        }
    }

    #ifdef DEPTH_MAPPER_DEBUG
        this->__debug = std::format("FPS: {:.2f} \nClickable: {} (ptr{:016X}) \nIs clickable pressed: {} \nDepth map size: {} \nRendered widgets: {}", 
                                        /* FPS */ 1/dt, 
                                        /* Clickable id*/(this->__clickable == nullptr ? "null" : this->__clickable->id), 
                                        /* Clickable ptr */reinterpret_cast<uintptr_t>(this->__clickable), 
                                        /* Is clickable pressed*/ (this->__clickable == nullptr ? false : this->__clickable->pressed()), 
                                        /* Size of depth map */ this->__mapped_widgets.size(), 
                                        /* Count of rendered widgets */ drawn_widgets);
    #endif

    LOGV("Finished");
    this->__mapped_widgets.clear();
}

#ifdef DEPTH_MAPPER_DEBUG
const std::string& depth_mapper::debug_information()
{
    return this->__debug;
}
#endif
