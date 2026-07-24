#include "text.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "zindex_manager.hpp"
#include "font.hpp"

using namespace iwa::widgets;

text::text(const text::params& data)
{
    this->data = data;
    this->data.apply_styles(*this);
}

iwa::canvas& text::get_canvas()
{
    return this->data;
}



void text::render(float dt)
{
    zindex_manager::get_instance()->push(this);
}

void text::draw(float dt)
{
    auto& params = this->data;

    if (!params.enabled) return;

    ImFont* font = nullptr;
    
    if (params.drawlist == nullptr) params.drawlist = ImGui::GetForegroundDrawList();
    if (params.font == "") font = params.drawlist->_Data->Font; 
    else font = iwa::get_font(params.font);
    
    ImVec2 text_size = font->CalcTextSizeA(params.size.get(), FLT_MAX, -1, params.text.c_str());
    auto& rect = params.compute_rect(text_size);
    if (params.shadow.distance > 0.0f)
    {
        auto shadow_position = ImVec2(cosl(params.shadow.angle), -sinl(params.shadow.angle)) * params.shadow.distance;
        params.drawlist->AddText(font, params.size.get(), rect.Min + shadow_position, params.shadow.color, params.text.c_str());
    }
    
    params.drawlist->AddText(font, params.size.get(), rect.Min, params.color, params.text.c_str());
}