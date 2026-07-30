#include "widget.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "button.hpp"
#include "config.hpp"
#include <unordered_map>
#include <vector>
#include <stacktrace>
#include <string>

using namespace iwa;

std::unordered_map<std::string, base_widget*> widgets = {{"000", nullptr}};

widget_manager::widget_manager() 
{
    LOGD("Widget manager");
}

void widget_manager::add(const std::string& id, base_widget *widget)
{
    widgets[id] = widget;
}

void widget_manager::remove(base_widget* widget)
{
    widgets.erase(widget->data()->id);
}

void iwa::widget_manager::remove(const std::string& id)
{
    widgets.erase(id);
}

size_t widget_manager::count()
{
    return widgets.size();
}

base_widget *iwa::widget_manager::get(const std::string& id)
{
    if (widgets.find(id) != widgets.end())
    {
        auto result = widgets[id];

        if (result == nullptr) LOGE("Widget with id %s is probably deleted.", id.data());
        return result;
    }
    else
    {
        LOGE("Couldn't find widget with id %s", id.data());
        return nullptr;
    }
}

void abstract_widget_params::destroy_widget()
{
    this->widget->destroy(); 
}

void abstract_widget_params::add_descendant(const std::string &id)
{
    if (!this->descendants)
        this->descendants = new widgets_set();

    this->descendants->emplace(id);

}
void abstract_widget_params::add_parent(const std::string &id)
{
    if (!this->parents)
        this->parents = new widgets_set();

    this->parents->emplace(id);
}

void abstract_widget_params::on_pre_draw(iwa::event<float>::callback_t fn)
{
    if (!this->pre)
        this->pre = new event<float>();
    this->pre->add(fn);
}

void abstract_widget_params::on_post_draw(iwa::event<float>::callback_t fn)
{
    if (!this->post)
        this->post = new event<float>();
    this->post->add(fn);
}