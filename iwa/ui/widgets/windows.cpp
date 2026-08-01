
#include "windows.hpp"

#include <vector>

#include "imgui_internal.h"
#include "depth_mapper.hpp"
#include "logger.hpp"
#include "utils.hpp"

using namespace iwa;

std::vector<head_window*> head_windows;

head_window::head_window(head_window_params* data) : abstract_widget(data)
{
    if (this->data()->__bounds.GetArea() == 0) this->data()->set_bounds({{0,0}, get_screen_resolution()});
    auto& rect = this->data()->compute_rect();
    if (this->data()->header_window != nullptr)
    {
        auto header_window = this->data()->header_window;
        auto header_window_data = header_window->data();

        header_window_data->id = this->data()->id + "_header";
        header_window_data->set_bounds(rect);
        header_window_data->pos = {0,0};
        header_window_data->anchor = {0,0};

        if (this->data()->drawflags & ImDrawFlags_RoundCornersTop == 0) header_window_data->drawflags = ImDrawFlags_RoundCornersTop;

        header_window_data->size.x = 1;

        if (this->data()->header_text != nullptr)
        {
            auto header_text = this->data()->header_text;
            auto header_text_data = header_text->data();

            header_text_data->id = this->data()->id + "_text";
            header_text_data->set_bounds(header_window_data->compute_rect());
            header_text_data->size.factor(header_window_data->compute_rect().Max.y - header_window_data->compute_rect().Min.y);
        }
    }
}

head_window::~head_window()
{
    delete data()->header_text;
    delete data()->header_window;
}

window::window(window_params* data) : abstract_widget(data)
{
    if (this->data()->__bounds.GetArea() == 0) this->data()->set_bounds({{0,0}, get_screen_resolution()});
    this->data()->compute_rect();
}

void window::push_to_depth_map(unsigned int parent_zindex)
{
    if (!data()->enabled) return;

    depth_mapper::get_instance()->push(reinterpret_cast<base_widget *>(this), data()->zindex + parent_zindex);
}

void window::render_descendants(unsigned int parent_zindex)
{
    if (!data()->enabled || !data()->descendants) return;
    auto mapper = depth_mapper::get_instance();
    auto& bounds = data()->compute_padding();
    auto& rect = data()->compute_rect();

    if (data()->clipping) mapper->push_clip(rect);

    for (auto &widget_id : *data()->descendants)
    {
        auto widget = widget_manager::get_instance()->get(widget_id);
        widget->data()->set_bounds(bounds);
        if (widget->data()->enabled) widget->render(data()->zindex + parent_zindex);
    }

    if (data()->clipping) mapper->pop_clip();

}

void window::render(unsigned int parent_zindex)
{
    data()->handle_focus();

    push(parent_zindex);
}

void head_window::push_to_depth_map(unsigned int parent_zindex)
{
    if (!data()->enabled) return;

    depth_mapper::get_instance()->push(reinterpret_cast<base_widget *>(this), data()->zindex + parent_zindex);
}


void head_window::render_descendants(unsigned int parent_zindex)
{
    if (!data()->enabled || !data()->descendants) return;
    auto mapper = depth_mapper::get_instance();
    auto bounds = data()->compute_padding();
    auto rect = data()->compute_rect();
    
    if (data()->clipping) mapper->push_clip(rect);

    if (data()->header_window)
    {
        auto hr_data = data()->header_window->data();
        bounds.Min.y += hr_data->scaled_size(hr_data->size).y + hr_data->size_px.y;
    }

    for (auto &widget_id : *data()->descendants)
    {
        auto widget = widget_manager::get_instance()->get(widget_id);
        widget->data()->set_bounds(bounds);
        if (widget->data()->enabled) widget->render(data()->zindex + parent_zindex);
    }

    if (data()->clipping) mapper->pop_clip();
}


void head_window::render(unsigned int parent_zindex)
{
    data()->handle_click(parent_zindex);
    data()->handle_dragging();
    data()->handle_focus();

    push(parent_zindex);
}

void window::draw(float dt)
{
    auto params = data();
    auto& rect = params->compute_rect();
    params->handle_focus();

    ImVec2 min = rect.Min;
    ImVec2 max = rect.Max;

    auto drawlist = ImGui::GetForegroundDrawList();

    drawlist->AddRectFilled(min, max, iwa::apply_alpha(params->color), params->rounding, params->drawflags);
}

void head_window::draw(float dt)
{
    auto params = data();
    if (!params->enabled) return;

    auto& rect = params->compute_rect();
    params->handle_focus();


    ImVec2 min = rect.Min;
    ImVec2 max = rect.Max;

    auto drawlist = ImGui::GetForegroundDrawList();

    drawlist->AddRectFilled(min, max, iwa::apply_alpha(params->color), params->rounding, params->drawflags);
    
    if (params->header_window != nullptr)
    {
        params->header_window->data()->set_bounds(rect);
        auto& header_rect = params->header_window->data()->compute_rect();
        params->header_window->draw(dt);

        if (params->header_line_color > 0)
        {
            drawlist->AddRect(
                {header_rect.Min.x + 1, header_rect.Max.y},
                {header_rect.Max.x - 1, header_rect.Max.y + params->header_line_thickness},
                iwa::apply_alpha(params->header_line_color));
        }

        if (params->header_text != nullptr)
        {
            params->header_text->data()->size.factor(header_rect.Max.y - header_rect.Min.y);
            params->header_text->data()->set_bounds(params->header_window->data()->compute_rect());
            if (params->header_text->data()->pre) params->header_text->data()->pre->call(dt);
            params->header_text->draw(dt);
            if (params->header_text->data()->post) params->header_text->data()->post->call(dt);

        }
    }

    if (params->outline_color > 0)
    {
        drawlist->AddRect(min, max, iwa::apply_alpha(params->outline_color), params->rounding, 1.0f, params->drawflags);
    }
}