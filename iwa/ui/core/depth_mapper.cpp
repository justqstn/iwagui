#include "depth_mapper.hpp"

#ifdef IWA_DEPTH_MAPPER_DEBUG
#include <format>
#endif

using namespace iwa;

depth_mapper::depth_mapper()
{
#ifdef IWA_DEPTH_MAPPER_DEBUG
    this->__debug = "None";
#endif
}

void depth_mapper::push_clip(const ImRect& rect)
{
    __clip_rect_current = rect;
    __clip_rect_current_id = __widgets_count;
}

void depth_mapper::pop_clip()
{
    __clip_rects.emplace(__clip_rect_current_id, std::pair(__widgets_count, __clip_rect_current));
    __clip_rect_current_id = 0;
    __clip_rect_current = {{0,0},{0,0}};
}

void depth_mapper::push(base_widget* widget, unsigned int parent_zindex)
{
    auto zindex = widget->data()->zindex + parent_zindex;
    __widgets_count += 1;
    if (!widget->alive())
    {
        this->__mapped_widgets[zindex].emplace_back(nullptr);
        return;
    }

    if (!this->__mapped_widgets.contains(zindex))
    {
        LOGV("Creating new ZIndex(%i) vector.", zindex);
        this->__mapped_widgets[zindex] = {};
    }

    LOGV("Object address: %p", widget);
    this->__mapped_widgets[zindex].emplace_back(widget);
}

void depth_mapper::input()
{
    LOGV("Input processing");
    if (this->__clickable != nullptr)
    {
        if (!this->__clickable->__down_called)
        {
            if (this->__clickable->down) this->__clickable->down->call();
            this->__clickable->__down_called = true;
        }
    }
}

void depth_mapper::render(float dt)
{
    LOGV("Starting drawing");
 
    unsigned int drawn_widgets = 0; 
    unsigned int end_rect_id = UINT_MAX;

    auto drawlist = ImGui::GetForegroundDrawList();

    for (auto zindex : __mapped_widgets)
    {
        LOGV("ZIndex %i", zindex.first);

        for (auto widget : zindex.second)
        {
            if (drawn_widgets == end_rect_id)
            {
                drawlist->PopClipRect();
                end_rect_id = UINT_MAX;
            }

            if (!widget)
            {
                drawn_widgets += 1;
                continue;
            }


            if (__clip_rects.contains(drawn_widgets))
            {
                
                auto& rect_entity = __clip_rects[drawn_widgets];
                drawlist->PushClipRect(rect_entity.second.Min, rect_entity.second.Max);
                end_rect_id = rect_entity.first;
            }

            if (widget->data()->pre) widget->data()->pre->call(dt);
            widget->draw(dt);
            if (widget->data()->post) widget->data()->post->call(dt);


            drawn_widgets += 1;
        }
    }

    #ifdef IWA_DEBUG_RENDERING
    for (auto zindex : __mapped_widgets)
    {

        for (auto widget : zindex.second)
        {
            auto font = drawlist->_Data->Font;
            auto data = widget->data();
            auto &rect = data->compute_rect();

            drawlist->AddRect(rect.Min, rect.Max, ImColor(255, 0, 0));
            auto size = 20;
            std::string str_data = std::format("{}: z:{}", data->id, data->zindex);
            auto text_size = font->CalcTextSizeA(size, FLT_MAX, -1, str_data.c_str()) / 2;


            auto anchor = rect.Min + ((rect.Max - rect.Min) * data->anchor);

            drawlist->AddLine({anchor.x, rect.Min.y}, {anchor.x, rect.Max.y}, ImColor(255,0,0));
            drawlist->AddLine({rect.Min.x, anchor.y}, {rect.Max.x, anchor.y}, ImColor(255,0,0));
            drawlist->AddText(font, size, rect.GetCenter() - text_size, ImColor(0, 0, 255), str_data.c_str());
        }
    }
    #endif

    #ifdef IWA_DEPTH_MAPPER_DEBUG
    __debug = std::format("FPS: {:.2f} \nClickable: {} (ptr{:016X}) \nIs clickable pressed: {} \nDepth map size: {} \nRendered widgets: {}",
                                /* FPS */ 1 / dt,
                                /* Clickable id*/ (this->__clickable == nullptr ? "null" : this->__clickable->id),
                                /* Clickable ptr */ reinterpret_cast<uintptr_t>(this->__clickable),
                                /* Is clickable pressed*/ (this->__clickable == nullptr ? false : this->__clickable->pressed()),
                                /* Size of depth map */ this->__mapped_widgets.size(),
                                /* Count of rendered widgets */ drawn_widgets);
    #endif

    LOGV("Finished");
    __mapped_widgets.clear();
    __clip_rects.clear();
    __widgets_count = 0; 
}

#ifdef IWA_DEPTH_MAPPER_DEBUG
const std::string& depth_mapper::debug_information()
{
    return this->__debug;
}
#endif
