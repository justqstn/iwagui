#include "input_controller.hpp"

using namespace iwa;

input_controller::input_controller()
{
    __clickable = nullptr;
    __last_clickable = nullptr;
}
    
void input_controller::handle_clickable(clickable* object)
{
    bool pressed = object->pressed();   
    if (object == __clickable && !pressed)
    {
        CALL_SIGNAL(__clickable->__release_signal);
        __clickable->__click_callback = false;
        __clickable = nullptr;
    }
    else if (__clickable == nullptr)
    {
        if (pressed) __clickable = object;
    }
    else if (object->__zindex >= __clickable->__zindex && pressed)
    {
        __clickable = object;
    }
}

void input_controller::__click_signal()
{
    if (__clickable && !__clickable->__click_callback)
    {
        __clickable->__click_callback = true;
        CALL_SIGNAL(__clickable->__click_signal)
    }
}   

void input_controller::update()
{
    if (__clickable && !__clickable->__click_callback)
    {
        __clickable->__click_callback = true;
        CALL_SIGNAL(__clickable->__click_signal)
    }
}