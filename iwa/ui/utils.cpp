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

ImVec2 iwa::scale(ImVec2 vec, ImVec2 parent)
{
    return vec * parent;
}

ImVec2 iwa::scale(float x, float y, float px, float py)
{
    return {x * py, y * px};
}

float iwa::scaled_float::get()
{
    if (this->scaled)
    {
        if (this->noncomputed < 0.0f) this->noncomputed = this->value;
        this->value = this->noncomputed * this->parent;
        this->scaled = false;
    }
    return this->value;
}

void iwa::scaled_float::set(float value, bool scaled)
{
    this->value = value;
    this->scaled = scaled;
}

void iwa::scaled_float::change_parent(float parent)
{
    this->scaled = true;
    this->parent = parent;  
}