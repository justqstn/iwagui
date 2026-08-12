#include "gui_controller.hpp"

#include "tween.hpp"

using namespace iwa;

gui_controller::gui_controller()
{
    __inputs = input_controller::get_instance();
    __renderer = render_controller::get_instance();
}

void gui_controller::tick(float dt)
{
    tween::tickall(dt);
    __renderer->push_ctxs();
    __inputs->update();
    __renderer->render(dt);
}