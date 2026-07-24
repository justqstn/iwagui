#include "utils.hpp"
#include "wtypes.h"
#include "logger.hpp"

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

iwa::scaled_float::scaled_float(float value)
{
    this->value = value;
    this->__is_saved_originals = true;
    this->__original = value;
}

float iwa::scaled_float::get()
{
    if (this->__is_recomputing)
    {
        // @todo Implement error handling (if factor is zero)
        this->value = this->__original * this->scaling_factor;

        this->__is_recomputing = false;
    }
    return this->value;
}

void iwa::scaled_float::set(float value)
{
    if (this->scaled)
    {   
        this->__original = value;
        this->__is_recomputing = true;
    }
    else
    {
        this->value = value;
    }
}

void iwa::scaled_float::factor(float factor)
{
    if (this->scaling_factor != factor)
    {
        this->scaled = true;
        this->scaling_factor = factor;  
        this->__is_recomputing = true;
    }
}

void iwa::scaled_float::scaling()
{
    this->scaled = true;
    this->__is_recomputing = true;
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
