#include "button.hpp"
#include "depth_mapper.hpp"

using namespace iwa;

button::button(button_params* data) : abstract_widget(data)
{}

void button::render(unsigned int parent_zindex)
{
    data()->handle_focus();
    data()->handle_click(parent_zindex);
    push(parent_zindex);
}

void button::push_to_depth_map(unsigned int parent_zindex)
{
    if (!data()->enabled) return;

    depth_mapper::get_instance()->push(reinterpret_cast<base_widget *>(this), data()->zindex + parent_zindex);
}

void button::draw(float dt)
{
    auto params = this->data();
    
    auto& rect = params->compute_rect();

    ImGui::GetForegroundDrawList()->AddRectFilled(rect.Min, rect.Max, params->color, params->rounding, params->drawflags);
}