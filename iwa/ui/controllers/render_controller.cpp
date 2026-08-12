#include "render_controller.hpp"

#include "logger.hpp"
#include "widget_controller.hpp"
#include "widget.hpp"
#include "macroses.hpp"
#include "context.hpp"

using namespace iwa;

render_controller::render_controller()
{   
    LOGD("render_controller::render_controller()")
    __pushed_widgets = {};
    __pushed_clip_rects = {};
}

void render_controller::push(widget* widget)
{
    auto zindex = widget->__zindex + widget->__ctx->__zindex;
    if (__pushed_widgets.contains(zindex))
        __pushed_widgets[zindex].emplace_back(widget);
    else
        __pushed_widgets[zindex] = { widget };

    __pushed_widgets_count++;
}

void render_controller::push_ctxs()
{
    if (!__enabled) return;

    for (auto& ctx_map_pair : context::get_ctx_map())
        if (ctx_map_pair.second->get_enabled()) ctx_map_pair.second->push();
}

void render_controller::render(float dt)
{
    if (!__enabled) return;
    CALL_SIGNAL(__pre_render_signal, dt)

    uint count = 0;
    uint pop_rect_zindex = -1;
    auto drawlist = ImGui::GetForegroundDrawList();
    for (auto& it : __pushed_widgets)
    {
        for (auto widget : it.second)
        {
            if (count == pop_rect_zindex)
            {
                drawlist->PopClipRect();
            }

            if (__pushed_clip_rects.contains(count))
            {
                auto& clip_rect = __pushed_clip_rects[count];
                auto& rect = clip_rect.second;
                drawlist->PushClipRect(rect.Min, rect.Max);
                pop_rect_zindex = count;
            }

            CALL_SIGNAL(widget->__pre_render_signal, dt)
            widget->render(dt);
            CALL_SIGNAL(widget->__post_render_signal, dt)

            count++;
        }
    }

    __pushed_widgets_count = 0;
    __pushed_widgets.clear();
    __pushed_clip_rects.clear();
    CALL_SIGNAL(__post_render_signal, dt)
}