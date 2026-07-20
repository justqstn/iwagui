#pragma once
// Internal utilites

#include "imgui.h"
#include "imgui_internal.h"
#include <vector>

#define BUILD(code) ({code}); code.init()

namespace iwa
{
    struct ImRect
    {
        ImVec2 min;
        ImVec2 max;
    };

    struct scaled_float
    {
        float value;
        bool scaled; 
        float parent;
        float noncomputed = -1.0f;
        float get();  
        void set(float value, bool scaled = false);
        void change_parent(float parent);
    };

    ImVec2 get_screen_resolution();
    
    #define SCREEN_X get_screen_resolution().x
    #define SCREEN_Y get_screen_resolution().y

    ImColor apply_alpha(ImColor src);
    ImColor apply_alpha(ImColor src, float alpha);
    ImVec2 scale(ImVec2 vec, ImVec2 parent);
    ImVec2 scale(ImVec2 vec);
    ImVec2 scale(float x, float y, float px, float py);
    ImVec2 scale(float x, float y);
}
