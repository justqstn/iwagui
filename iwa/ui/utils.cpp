#include "utils.hpp"
#include "wtypes.h"
#include "logger.hpp"

ImVec2 iwa::get_screen_resolution()
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    return {desktop.right, desktop.bottom};
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