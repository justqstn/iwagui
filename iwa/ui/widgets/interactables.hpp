#pragma once

#include "windows.hpp"

namespace iwa
{
    class button : public widget
    {
    public:
        struct params : abstract_window_params, stylable<params, button>, clickable 
        {
            friend button;
            bool pressed() override;
        };
        
        button(const params& data);
        void render() override;
        canvas& get_canvas() override;
        params data;
    protected:
        void draw(float dt) override;
    };
}