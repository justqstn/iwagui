#pragma once

#include "utils.hpp"
#include "canvas.hpp"
#include "attributes.hpp"

#include <string>

namespace iwa
{
    class text;
    struct text_params : abstract_widget_params, text_canvas, stylable<text_params, text>, focusable
    {
    public:
        struct shadow_params
        {
            ImU32 color = ImColor(0, 0, 0, 255);
            long double angle; // in radians
            float thickness;
            float distance;
            bool is_glow;
        };
        friend text;
        std::string text;
        std::string font;
        shadow_params* shadow = nullptr; 
        
        void update_size();

    private:
        ImVec2& calculate_text_size() override;
        ImFont *get_font();
    };

    class text : public abstract_widget<text_params>
    {
    public:
        text(text_params* data);
        ~text();
        void render(unsigned int parent_zindex) override;
        void draw(float dt) override;
    protected:
        void push_to_depth_map(unsigned int parent_zindex) override;
    };
}