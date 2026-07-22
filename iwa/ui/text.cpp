#include "text.hpp"
#include "imgui.h"
#include "imgui_internal.h"

using namespace iwa::widgets;

text::text(const text::params& data)
{
    this->data = data;
}

void text::render(float dt)
{
    auto& params = this->data;

    if (params.drawlist == nullptr) params.drawlist = ImGui::GetForegroundDrawList();
    if (this->data.font == nullptr) params.font = params.drawlist->_Data->Font;
    

    ImVec2 text_size = params.font->CalcTextSizeA(params.size.get(), FLT_MAX, -1, params.text.c_str());
    auto& rect = params.compute_rect(text_size);

    params.drawlist->AddText(params.font, params.size.get(), rect.Min, params.color, params.text.c_str());
}