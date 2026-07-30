#pragma once

#include "windows.hpp"

namespace iwa
{
    class button;
    struct button_params : abstract_window_params, stylable<button_params, button>, clickable
    {
        friend button;
    };
    class button : public abstract_widget<button_params>
    {
    public:
        button(button_params* data);
        void render(unsigned int parent_zindex) override;
        void draw(float dt) override;
    protected:
        void push_to_depth_map(unsigned int parent_zindex) override;
    };
}