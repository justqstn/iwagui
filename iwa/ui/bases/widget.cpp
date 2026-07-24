#include "widget.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "config.hpp"

#include <vector>

using namespace iwa;

std::vector<widget*> widgets;

unsigned int widget::get_id()
{
    if (!this->__id_initialized)
    {
        this->__id_initialized = true;
        widgets.emplace_back(this);
        this->__id = widgets.size();
    }
    return this->__id;
}

void widget::params::toggle()
{
    this->enabled = !this->enabled;
}

void widget::params::disable()
{
    this->enabled = false;
}

void widget::params::enable()
{
    this->enabled = true;
}

widget::widget()
{
    this->__id = this->get_id();
    
    LOGD("Widget with unique id %i added.", this->__id);
}

widget* widget::get(unsigned int id)
{
    return widgets[id-1];
}