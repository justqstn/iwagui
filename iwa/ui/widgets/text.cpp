#include "text.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "zindex_manager.hpp"
#include "font.hpp"

using namespace iwa;

text::text(const text::params& data)
{
    this->data = data;
    this->data.apply_styles(*this);
    this->__zindex = this->data.zindex;
}

iwa::canvas& text::get_canvas()
{
    return this->data;
}

ImFont* text::params::get_font()
{
    return this->font == "" ? this->drawlist->_Data->Font : iwa::get_font(this->font);
}

bool text::params::focused()
{
    return this->compute_rect(this->get_text_size()).Contains(ImGui::GetMousePos());
}

ImVec2 text::params::get_text_size()
{
    return this->get_font()->CalcTextSizeA(this->size.get(), FLT_MAX, -1, this->text.c_str());
}

void text::render()
{
    if (this->data.enabled) zindex_manager::get_instance()->push(this);
}

void text::draw(float dt)
{
    auto& params = this->data;

    if (!params.enabled) return params.clear_focus();
    if (params.drawlist == nullptr) params.drawlist = ImGui::GetForegroundDrawList();

    ImFont* font = params.get_font();
    ImVec2 text_size = params.get_text_size();
    auto& rect = params.compute_rect(text_size);
    params.handle_focus();

    if (params.shadow.distance > 0.0f)
    {
        auto shadow_position = ImVec2(cosl(params.shadow.angle), -sinl(params.shadow.angle)) * params.shadow.distance;
        params.drawlist->AddText(font, params.size.get(), rect.Min + shadow_position, params.shadow.color, params.text.c_str());
    }
    
    params.drawlist->AddText(font, params.size.get(), rect.Min, params.color, params.text.c_str());
}