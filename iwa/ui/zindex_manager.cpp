#include "zindex_manager.hpp"

using namespace iwa;

zindex_manager::zindex_manager() {}

void zindex_manager::push(widget* object)
{
    auto zindex = object->__zindex + object->__parent_zindex;

    if (!this->zindex_queue.contains(zindex))
    {
        LOGV("Creating new ZIndex(%i) vector.", zindex);
        this->zindex_queue[zindex] = {};
    }

    LOGV("Object address: %p", object);
    this->zindex_queue[zindex].emplace_back(object);
}

void zindex_manager::render(float dt)
{
    LOGV("Starting");

    for (auto zindex : this->zindex_queue)
    {
        LOGV("ZIndex %i", zindex.first);

        for (auto object : zindex.second)
        {            
            object->draw(dt);
        }
    }

    LOGV("Finished");
    this->zindex_queue.clear();
}