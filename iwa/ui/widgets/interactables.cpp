#include "interactables.hpp"
#include "zindex_manager.hpp"

using namespace iwa;

button::button(const button::params& data)
{
    this->data = data;
    this->data.apply_styles(*this);
    this->__zindex = this->data.zindex;
}

bool button::params::pressed()
{
    bool focused = this->compute_rect().Contains(ImGui::GetMousePos());
    bool pressed = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    return (pressed && this->__pressed) || (focused && pressed); 
}

void button::render()
{
    if (!this->data.enabled) return;
    auto& params = this->data;

    params.handle_click(this->__zindex + this->__parent_zindex);
    zindex_manager::get_instance()->push(this);
}

canvas& button::get_canvas()
{
    return this->data;
}

void button::draw(float dt)
{
    auto& params = this->data;
    
    auto& rect = params.compute_rect();
    params.pre.call(dt);

    ImGui::GetForegroundDrawList()->AddRectFilled(rect.Min, rect.Max, params.color);
    params.post.call(dt);
}