#include "widget_controller.hpp"
#include "logger.hpp"
#include "widget.hpp"
#include <stdexcept>
#include <format>

using namespace iwa;

widget_controller::widget_controller()
{
    __widgets = {};
    __widget_tags = {};
    LOGD("widget_controller::widget_controller()")
}

void widget_controller::add(widget* widget)
{
    const auto& id = widget->__id;

    if (__widgets.contains(id))
        LOGW("There is already widget with id \"%s\".", id.data())

    LOGD("Added new widget: id %s    ptr %p", id.data(), widget);
    __widgets[id] = widget;

    auto widget_tags = widget->get_tags();
    if (widget_tags)
    {
        for (const auto& tag : *widget_tags) 
        {
            if (!__widget_tags.contains(tag))
                __widget_tags[tag] = {};

            __widget_tags[tag].emplace(id);
        }
    }
}

void widget_controller::remove(widget* widget)
{
    if (__widgets.contains(widget->__id))
    {
        __widgets.erase(widget->__id);
        delete widget;
    }
    else LOGW("Widget with id \"%s\" is already removed.", widget->__id.data())
}
        
widget* widget_controller::get(const std::string& id)
{
    if (__widgets.contains(id))
    {
        auto widget = __widgets[id];
        if (widget == nullptr)
            LOGE("Widget with id \"%s\" is null.")

        return widget;
    }
    else
    {
        LOGE("Widget with id \"%s\" is deleted or not created.", id.data())
        return nullptr;
    }
}

const std::unordered_set<std::string>& widget_controller::get_tagged(const std::string& tag)
{
    if (!__widget_tags.contains(tag))
        LOGE("There is no widgets with tag \"%s\".", tag.data())

    return __widget_tags[tag];
}
