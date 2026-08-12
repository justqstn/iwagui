#pragma once

#include "iwagui.hpp"


class ui : public iwa::singleton<ui>
{
public:
    ui();
    void render(float dt);
    void tick(float dt);
    void setstyle();
    void init();
    iwa::signal<> on_toggle;
    iwa::signal<> on_closed;
};  