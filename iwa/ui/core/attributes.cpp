#include "attributes.hpp"
#include "input_controller.hpp"

using namespace iwa;

bool hoverable::hovered()
{
    return compute_rect().Contains(ImGui::GetMousePos());
}

interactible::~interactible() {}

hoverable::~hoverable()
{
    LOGD("hoverable::~hoverable")
    delete __enter_signal;
    delete __leave_signal;
}

void hoverable::handle_interact()
{
    if (!__hovering) return;
    if (hovered())
    {
        if (!__enter_callback)
        {
            __enter_callback = true;
            CALL_SIGNAL(__enter_signal);
        }
    }
    else if (__enter_callback)
    {
        __enter_callback = false;
        CALL_SIGNAL(__leave_signal);
    }
}

clickable::~clickable()
{
    LOGD("clickable::~clickable")
    delete this->__click_signal; 
    delete this->__release_signal; 
}

bool clickable::pressed()
{
    bool hovered = compute_rect().Contains(ImGui::GetMousePos());
    bool pressed = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool holding = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool result = (pressed && hovered) || (holding && __click_state);
    __click_state = result; 
    return result;
}

void clickable::handle_interact()
{
    input_controller::get_instance()->handle_clickable(this);
}


bool draggable::pressed()
{
    bool hovered = __drag_canvas->compute_rect().Contains(ImGui::GetMousePos());
    bool pressed = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool holding = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool result = (pressed && hovered) || (holding && __click_state);
    __click_state = result; 
    return result;
}


void draggable::handle_interact()
{   
    if (!__dragging) return;
    if (pressed() && __click_callback)
    {           
        if (__click_captured)
        {
            auto delta = ImGui::GetMousePos() - __click_pos;
            if (!__drag_callback && delta.x != 0 && delta.y != 0)
            {
                __drag_callback = true;
                CALL_SIGNAL(__start_dragging_signal)
            } 
            __pos_px += delta;
            __recomputing = true;
        }
        __click_pos = ImGui::GetMousePos();
        __click_captured = true;
    }
    else
    {
        if (__drag_callback) 
        {
            __drag_callback = false;
            CALL_SIGNAL(__end_dragging_signal)
        }
        __click_pos = {0, 0};
        __click_captured = false;
    }
}

draggable::~draggable()
{
    LOGD("draggable::~draggable()")
    delete this->__start_dragging_signal;
    delete this->__end_dragging_signal;
}

void bounded_draggable::handle_interact()
{   
    if (!__dragging) return;
    if (pressed() && __click_callback)
    {           

        if (__click_captured)
        {
            auto delta = ImGui::GetMousePos() - __click_pos;

            if (!__drag_callback && delta.x != 0 && delta.y != 0)
            {
                __drag_callback = true;
                CALL_SIGNAL(__start_dragging_signal)
            }

            if (__drag_bounds->Min == ImVec2(0, 0) && __drag_bounds->Max == ImVec2(0, 0))
            {
                __pos_px += delta;
                __recomputing = true;
            }
            else
            {
                auto saved_delta = delta;

                __pos_px += delta;
                this->recompute();
                auto &rect = compute_rect();

                side_t hitted_side = side_t::none;

                if (rect.Min.x < __drag_bounds->Min.x)
                {
                    delta.x -= (rect.Min.x - __drag_bounds->Min.x);
                    hitted_side |= side_t::left;
                }
                else if (rect.Max.x > __drag_bounds->Max.x)
                {
                    delta.x -= (rect.Max.x - __drag_bounds->Max.x);
                    hitted_side |= side_t::right;
                }
                if (rect.Min.y < __drag_bounds->Min.y)
                {
                    delta.y -= (rect.Min.y - __drag_bounds->Min.y);
                    hitted_side |= side_t::upper;
                }
                else if (rect.Max.y > __drag_bounds->Max.y)
                {
                    delta.y -= (rect.Max.y - __drag_bounds->Max.y);
                    hitted_side |= side_t::bottom;
                }

                if (hitted_side != side_t::none)
                    CALL_SIGNAL(__collide_with_side_signal, hitted_side);

                if (delta != saved_delta)
                {
                    __pos_px -= saved_delta;
                    __pos_px += delta;
                    __recomputing = true;
                }
            }
        }
        __click_pos = ImGui::GetMousePos();
        __click_captured = true;
    }
    else
    {
        if (__drag_callback) 
        {
            __drag_callback = false;
            CALL_SIGNAL(__end_dragging_signal)
        }
        __click_pos = {0, 0};
        __click_captured = false;
    }
}


bounded_draggable::~bounded_draggable()
{
    LOGD("bounded_draggable::~bounded_draggable()")
    delete this->__collide_with_side_signal;
}
