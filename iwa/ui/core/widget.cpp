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



widget_manager::widget_manager() 
{
    this->__widgets = {};
    this->parent_queue = {};
    LOGD("Widget manager");
}

void widget_manager::add(const std::string& id, base_widget *widget)
{
    if (__widgets.contains(id)) LOGW("There was a widget with the same ID \"%s\", overwriting it.", id.data());
    __widgets[id] = widget;
}

void widget_manager::remove(base_widget* widget)
{
    __widgets.erase(widget->data()->id);
}

void iwa::widget_manager::remove(const std::string& id)
{
    __widgets.erase(id);
}

size_t widget_manager::count()
{
    return __widgets.size();
}

base_widget *iwa::widget_manager::get(const std::string& id)
{
    if (__widgets.find(id) != __widgets.end())
    {
        auto result = __widgets[id];

        if (result == nullptr) LOGE("Widget with id %s is probably deleted.", id.data());
        return result;
    }
    else
    {
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
        this->descendants = new widgets_id_set();    

    this->descendants->emplace(id);
}

void abstract_widget_params::add_parent(const std::string &id)
{
    if (!this->parents)
        this->parents = new widgets_id_set();

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