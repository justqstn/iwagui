#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include "typedefs.hpp"

#include <vector>
#include <string>

// goofy ah macros
#define DEBUG_RECT(rect) ImGui::GetForegroundDrawList()->AddRect(rect.Min, rect.Max, ImColor(255,0,0));

namespace iwa
{
    ImVec2 get_screen_resolution();

    color_t apply_alpha(color_t src);
    color_t apply_alpha(color_t src, float alpha);

    std::string random_id();
}

long double rad(long double deg);
long double deg(long double rad);
ImVec2 ratio(ImVec2 vec, ImVec2 aspects);
ImRect ratio(const ImRect& rect, ImVec2 aspects);
