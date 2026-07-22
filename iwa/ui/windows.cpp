
#include "windows.hpp"

#include <vector>

#include "imgui_internal.h"
#include "logger.hpp"
#include "utils.hpp"

using namespace iwa::widgets;

// silly macro cuz why not
#define RENDER(widget, dt) widget->params.pre.call(dt); widget->render(dt); widget->params.post.call(dt);

std::vector<begin*> begins;

begin::begin(const begin::params& data)
{
    this->data = data;
    this->data.set_bounds({{0,0}, get_screen_resolution()});
    if (this->data.header_thickness.scaled) this->data.header_thickness.factor(this->data.size.y);
    begins.emplace_back(this);
}

void begin::params::scaling()
{
    this->pos_scaling();
    this->size_scaling();
    if (this->header_thickness.value != 0.0f)
    {
        this->header_thickness.scaling();
    }
}

void begin::render(const float dt)
{
    auto& params = this->data;
    auto& rect = params.compute_rect();
    ImVec2 min = rect.Min;
    ImVec2 max = rect.Max;
    auto drawlist = ImGui::GetForegroundDrawList();
    params.pre.call(dt);

    drawlist->PushClipRect(min, max);
    drawlist->AddRectFilled(min, max, iwa::apply_alpha(params.color), params.rounding);

    params.header_thickness.factor(params.size.y);
    float header_thickness = params.header_thickness.get();

    if (header_thickness > 0)
    {
        drawlist->AddRectFilled(min, {max.x, min.y + header_thickness}, iwa::apply_alpha(params.header_color), 0, 0);
    }

    if (params.header_line_color > 0)
    {
        drawlist->AddRect(
            {min.x + 1, min.y + header_thickness}, 
            {max.x - 1, min.y + header_thickness},
            iwa::apply_alpha(params.header_line_color)
        );
    }

    if (params.outline_color > 0)
    {
        drawlist->AddRect(min, max, iwa::apply_alpha(params.outline_color), 0.0f, ImDrawFlags_None, 1.0f);
    }

    drawlist->PopClipRect();
    params.post.call(dt);
}

