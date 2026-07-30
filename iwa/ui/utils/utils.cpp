#include "utils.hpp"
#include "wtypes.h"
#include "logger.hpp"
#include <random>

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

iwa::scaled_float::scaled_float()
{
    this->value = 0.0f;
    this->value_px = 0.0f;
    __recomputing = true;
}

iwa::scaled_float::scaled_float(float value)
{
    this->value = value;
    this->value_px = 0.0f;
    __recomputing = true;
}

iwa::scaled_float::scaled_float(float value, float value_px)
{
    this->value = value;
    this->value_px = value_px;
    __recomputing = true;
}


float iwa::scaled_float::compute()
{
    if (__recomputing)
    {
        // @todo Implement error handling (if factor is zero)
        __computed = this->value_px + this->value * this->scaling_factor;
        __recomputing = false;
    }
    return __computed;
}

void iwa::scaled_float::set(float value)
{
    if (this->value != value)
    {
        this->value = value;
        __recomputing = true;
    }
}

void iwa::scaled_float::set_px(float value_px)
{
    if (this->value_px != value_px)
    {
        this->value_px = value_px;
        __recomputing = true;
    }
}

void iwa::scaled_float::factor(float factor)
{
    if (this->scaling_factor != factor)
    {
        this->scaling_factor = factor;  
        __recomputing = true;
    }
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
    return deg * M_PI / 180.0; 
}

long double deg(long double rad)
{
    return (rad / M_PI) * 180.0;
}

std::string iwa::random_id()
{
    static std::mt19937 mt{};
    return "anon"+std::to_string(mt());
}