#include "utils.hpp"
#include "wtypes.h"
#include "logger.hpp"

#include <random>

#define _USE_MATH_DEFINES
#include <cmath>

ImVec2 iwa::get_screen_resolution()
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    return {(float)desktop.right, (float)desktop.bottom};
}

ImColor iwa::apply_alpha(ImColor src, float alpha)
{  
    if (alpha != 0.0f || alpha != 1.0f) return (src & 0x00FFFFFF) | ((int)(((int)(src >> IM_COL32_A_SHIFT) & 0xFF) * alpha) << 24);
    return src;
}

ImColor iwa::apply_alpha(ImColor src)
{
    return iwa::apply_alpha(src, ImGui::GetStyle().Alpha);
}

ImVec2 ratio(ImVec2 vec, ImVec2 aspects)
{
    auto resolution = iwa::get_screen_resolution();
    float aspect_ratio;
    if (resolution.x >= resolution.y)
    {
        aspect_ratio = resolution.y / resolution.x;
        return {vec.x * aspect_ratio * aspects.y / aspects.x, vec.y};
    }   
    else
    {   
        aspect_ratio = resolution.x / resolution.y;
        return {vec.x, vec.y * aspect_ratio * aspects.x / aspects.y};
    }
}

ImRect ratio(const ImRect& rect, ImVec2 aspects)
{
    auto resolution = iwa::get_screen_resolution();
    
    return {ratio(rect.Min, aspects), ratio(rect.Max, aspects)};
}


long double rad(long double deg)
{
    return deg * std::numbers::pi / 180.0; 
}

long double deg(long double rad)
{
    return (rad / std::numbers::pi) * 180.0;
}

std::string iwa::random_id()
{
    static std::mt19937 mt{};
    return "anon"+std::to_string(mt());
}