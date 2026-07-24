#pragma once 

#include "singleton.hpp"
#include "widget.hpp"
#include "attributes.hpp"

#include <map>
#include <vector>

namespace iwa
{
    class zindex_manager : public singleton<zindex_manager>
    {
    public:
        friend clickable;

        zindex_manager();
        void input();
        void render(float dt);
        void push(widget* object);
    private:
        std::map<u_short, std::vector<widget*>> __zindex_queue;
        clickable* __clickable = nullptr;
        u_short __clickable_zindex;
        bool __clickable_pressed;
        void click_event(clickable* object, u_short zindex, bool pressed);
    };
}