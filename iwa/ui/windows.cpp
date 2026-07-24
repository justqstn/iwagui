
#include "windows.hpp"

#include <vector>

#include "imgui_internal.h"
#include "zindex_manager.hpp"
#include "logger.hpp"
#include "utils.hpp"

using namespace iwa::widgets;

std::vector<head_window*> head_windows;

head_window::head_window(const head_window::params& data)
{
    this->data = data;
    if (this->data.__bounds.GetArea() == 0) this->data.set_bounds({{0,0}, get_screen_resolution()});
    this->data.compute_rect();
    this->data.header_thickness.factor(this->data.size.y);
    this->__zindex = this->data.zindex;
}

window::window(const window::params& data)
{
    this->data = data;
    if (this->data.__bounds.GetArea() == 0) this->data.set_bounds({{0,0}, get_screen_resolution()});
    this->data.compute_rect();
    this->__zindex = this->data.zindex;
}

iwa::canvas& window::get_canvas()
{
    return this->data;
}

iwa::canvas& head_window::get_canvas()
{
    return this->data;
}

void head_window::params::scaling()
{
    this->pos_scaling();
    this->size_scaling();
    this->padding_scaling();
    if (this->header_thickness.value != 0.0f)
    {
        this->header_thickness.scaling();
    }
}

bool abstract_window_params::focused() { return this->compute_rect().Contains(ImGui::GetMousePos()); }

void window::render(float dt)
{
    auto& params = this->data;
    if (!params.enabled) return;

    auto zindexator = zindex_manager::get_instance();

    zindexator->push(this);

    for (auto object_id : this->__widgets)
    {
        auto object = widget::get(object_id);
        object->get_canvas().set_bounds(params.compute_padding());
        zindexator->push(object);
    }
}

void head_window::render(float dt)
{
    auto& params = this->data;
    if (!params.enabled) return;

    auto zindexator = zindex_manager::get_instance();

    zindexator->push(this);

    for (auto object_id : this->__widgets)
    {
        auto object = widget::get(object_id);
        object->get_canvas().set_bounds(params.compute_padding());
        zindexator->push(object);
    }
}

void window::draw(float dt)
{
    auto& params = this->data;
    if (!params.enabled) return params.clear_focus();

    auto& rect = params.compute_rect();
    params.handle_focus();

    ImVec2 min = rect.Min;
    ImVec2 max = rect.Max;

    auto drawlist = ImGui::GetForegroundDrawList();
    params.pre.call(dt);

    if (params.clipping) drawlist->PushClipRect(min, max);
    drawlist->AddRectFilled(min, max, iwa::apply_alpha(params.color), params.rounding);

    for (auto object_id : this->__widgets)
    {
        auto object = widget::get(object_id);
        object->get_canvas().set_bounds(rect);
        object->render(dt);
    }

    if (params.clipping) drawlist->PopClipRect();
    params.post.call(dt);
}

void head_window::draw(float dt)
{
    auto& params = this->data;
    if (!params.enabled) return;

    auto& rect = params.compute_rect();
    params.handle_focus();


    ImVec2 min = rect.Min;
    ImVec2 max = rect.Max;

    auto drawlist = ImGui::GetForegroundDrawList();
    params.pre.call(dt);

    if (params.clipping) drawlist->PushClipRect(min, max);
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

    if (params.clipping) drawlist->PopClipRect();
    params.post.call(dt);
}