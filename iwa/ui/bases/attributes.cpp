#include "attributes.hpp"
#include "zindex_manager.hpp"

using namespace iwa;

void parent_widget::add_widget(unsigned int id)
{
    this->__widgets.emplace_back(id);
}

void parent_widget::add_widget(widget& object)
{
    this->__widgets.emplace_back(object.get_id());
    object.__parent_zindex = this->__zindex + 1;
}

void focusable::clear_focus()
{
    this->__entered = false;
}

void focusable::handle_focus()
{
    if (this->focused())
    {
        if (!this->__entered)
        {
            this->__entered = true;
            this->enter.call();
        }
    }
    else if (this->__entered) 
    {
        this->__entered = false;
        this->leave.call();
    }
}

void clickable::handle_click(u_short zindex)
{
    auto zindexer = zindex_manager::get_instance();
    bool is_pressed = this->pressed();

    if (zindexer->__clickable == this)
    {
        if (!is_pressed)
        {
           
            this->up.call();
            zindexer->__clickable = nullptr;
            zindexer->__clickable_pressed = false;
            zindexer->__clickable_zindex = 0;
        }
    }
    else if (is_pressed && !this->__pressed)
    {
        zindexer->click_event(this, zindex, true);
        this->__pressed = true;
    }

    if (!is_pressed)
    {
        this->__pressed = false;
    }
    
}