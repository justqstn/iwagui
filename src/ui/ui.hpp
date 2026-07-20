#pragma once

#include "../utils.hpp"
#include "event.hpp"

namespace iwa
{
    class ui : public iwa::singleton<ui>
    {
    public:
        ui();
        void render(float dt);
        void tick(float dt);
        void setstyle();
        void init();
        iwa::event<> on_toggle;
    };
}