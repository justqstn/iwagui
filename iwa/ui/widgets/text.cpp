#include "text.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "depth_mapper.hpp"
#include "font.hpp"

using namespace iwa;

text::text(text_params* data) : abstract_widget(data)
{
}

ImFont* text_params::get_font()
{
    return this->font == "" ? ImGui::GetForegroundDrawList()->_Data->Font : iwa::get_font(this->font);
}

ImVec2& text_params::calculate_text_size()
{
    auto new_size = get_font()->CalcTextSizeA(this->size.compute(), FLT_MAX, -1, this->text.c_str());
    if (new_size != __text_size)
    {
        __text_size = new_size;
        __recomputing = true;
    }
    return __text_size;
}

void text::push_to_depth_map(unsigned int parent_zindex)
{
    if (!data()->enabled) return;

    depth_mapper::get_instance()->push(reinterpret_cast<base_widget *>(this), data()->zindex + parent_zindex);
}

void text::render(unsigned int parent_zindex)
{
    data()->handle_focus(); 
    push(parent_zindex);
}

text::~text()
{
    delete data()->shadow;
}

void text::draw(float dt)
{
    auto params = this->data();

    if (!params->enabled) return params->clear_focus();
    auto drawlist = ImGui::GetForegroundDrawList();

    ImFont* font = params->get_font();
    auto& rect = params->compute_rect();

    if (params->shadow)
    {
        auto shadow_position = ImVec2(cosl(params->shadow->angle), -sinl(params->shadow->angle)) * params->shadow->distance;
        drawlist->AddText(font, params->size.compute(), rect.Min + shadow_position, params->shadow->color, params->text.c_str());
    }
    
    drawlist->AddText(font, params->size.compute(), rect.Min, params->color, params->text.c_str());
}