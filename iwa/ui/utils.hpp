#pragma once
// Internal utilites

#include "imgui.h"
#include "imgui_internal.h"
#include <vector>

#define DEBUG_RECT(rect) ImGui::GetForegroundDrawList()->AddRect(rect.Min, rect.Max, ImColor(255,0,0));

namespace iwa
{
    struct scaled_float
    {
    public:
        scaled_float(float value = 0.0f);
        float value;
        float scaling_factor;
        bool scaled; 
        
        float get();  // @todo Get rid of getters
        void scaling();
        void set(float value);
        void factor(float factor);
    private:
        float __original;
        bool __is_recomputing = false;
        bool __is_saved_originals = false;
    };

    ImVec2 get_screen_resolution();
    
    #define SCREEN_X get_screen_resolution().x
    #define SCREEN_Y get_screen_resolution().y

    ImColor apply_alpha(ImColor src);
    ImColor apply_alpha(ImColor src, float alpha);
}
