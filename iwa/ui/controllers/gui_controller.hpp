#pragma once

#include "singleton.hpp"

#include "input_controller.hpp"
#include "render_controller.hpp"

namespace iwa
{
    class gui_controller final : public singleton<gui_controller>
    {
    public:
        gui_controller();
        void tick(float dt);
    private:
        input_controller* __inputs;
        render_controller* __renderer;
    };
}