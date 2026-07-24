#include "zindex_manager.hpp"

using namespace iwa;

zindex_manager::zindex_manager() {}

void zindex_manager::push(widget* object)
{
    auto zindex = object->__zindex + object->__parent_zindex;

    if (!this->__zindex_queue.contains(zindex))
    {
        LOGV("Creating new ZIndex(%i) vector.", zindex);
        this->__zindex_queue[zindex] = {};
    }

    LOGV("Object address: %p", object);
    this->__zindex_queue[zindex].emplace_back(object);
}

void zindex_manager::render(float dt)
{
    LOGV("Clickable processing");
    if (this->__clickable != nullptr)
    {
        if (!this->__clickable_pressed)
        {
            this->__clickable->down.call();
            this->__clickable_pressed = true;
        }
    }

    LOGV("Starting drawing");

    for (auto zindex : this->__zindex_queue)
    {
        LOGV("ZIndex %i", zindex.first);

        for (auto object : zindex.second)
        {            
            object->draw(dt);
        }
    }

    LOGV("Finished");
    this->__zindex_queue.clear();
}

void zindex_manager::click_event(clickable* object, u_short zindex, bool pressed)
{
    if (this->__clickable == nullptr)
    {
        if (pressed)
        {
            this->__clickable = object;
            this->__clickable_zindex = zindex;
        }
    }
    else if (zindex >= this->__clickable_zindex)
    {
        this->__clickable = object;
        this->__clickable_zindex = zindex;
    }
}