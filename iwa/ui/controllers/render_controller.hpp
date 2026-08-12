#pragma once

#include <map>
#include <unordered_set>
#include <vector>

#include "typedefs.hpp"
#include "singleton.hpp"
#include "widget.hpp"

namespace iwa
{
    template<class widget_t>
    class abstract_widget;
    using widget = abstract_widget<void>;
    class render_controller final : public singleton<render_controller>
    {
        TOGGEABLE_COMPONENT(enabled, true)
        SIGNAL_COMPONENT(float, pre_render, nullptr)
        SIGNAL_COMPONENT(float, post_render, nullptr)
    public:
        render_controller();

        void push_clip(const rect_t& rect);
        void pop_clip();
        void push(widget* widget);
        void push_ctxs();
        void render(float dt);

    private:
        uint __pushed_widgets_count;
        rect_t* __current_rect;
        std::map<uint, std::vector<widget*>> __pushed_widgets;
        std::map<uint, std::pair<uint, rect_t>> __pushed_clip_rects;
    };
}