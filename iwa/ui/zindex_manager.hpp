#pragma once 

#include "singleton.hpp"
#include "bases.hpp"

#include <map>
#include <vector>

namespace iwa
{
    class zindex_manager : public singleton<zindex_manager>
    {
    public:
        zindex_manager();
        void render(float dt);
        void push(widget* object);
    private:
        std::map<u_short, std::vector<widget*>> zindex_queue;
    };
}