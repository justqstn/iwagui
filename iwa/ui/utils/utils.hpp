#pragma once
// Internal utilites

#include "imgui.h"
#include "imgui_internal.h"
#include <vector>
#include <string>

// goofy ah macros
#define DEBUG_RECT(rect) ImGui::GetForegroundDrawList()->AddRect(rect.Min, rect.Max, ImColor(255,0,0));

namespace iwa
{
    // @todo need to refactoring
    struct scaled_float
    {
    public:
        scaled_float();
        scaled_float(float scaled);
        scaled_float(float scaled, float value_px);
        float value;
        float value_px;
        float scaling_factor;
        
        float compute();
        void set(float value);
        void set_px(float value_px);
        void factor(float factor);
    private:
        float __computed;
        bool __recomputing;
    };

    ImVec2 get_screen_resolution();
    
    #define SCREEN_X get_screen_resolution().x
    #define SCREEN_Y get_screen_resolution().y

    ImColor apply_alpha(ImColor src);
    ImColor apply_alpha(ImColor src, float alpha);

    std::string random_id();
}

long double rad(long double deg);
long double deg(long double rad);
ImVec2 ratio(ImVec2 vec, ImVec2 aspects);
ImRect ratio(const ImRect& rect, ImVec2 aspects);
