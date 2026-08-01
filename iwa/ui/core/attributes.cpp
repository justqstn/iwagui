#include "attributes.hpp"
#include "depth_mapper.hpp"

/**
 * i have to implement methods for safely setting positions/sizes
 * also fix draggables
 * 
 */

using namespace iwa;

void focusable::clear_focus()
{
    this->__entered = false;
}

bool focusable::focused()
{
    return compute_rect().Contains(ImGui::GetMousePos());
}

void focusable::handle_focus()
{
    if (this->focused())
    {
        if (!this->__entered)
        {
            this->__entered = true;
            if (this->enter) this->enter->call();
        }
    }
    else if (this->__entered) 
    {
        this->__entered = false;
        if (this->leave) this->leave->call();
    }
}

void focusable::on_enter(event<>::callback_t fn)
{
    if (!this->enter) this->enter = new event<>();
    this->enter->add(fn);
}

void focusable::on_leave(event<>::callback_t fn)
{
    if (!this->leave) this->leave = new event<>();
    this->leave->add(fn);
}

void clickable::on_pressed(event<>::callback_t fn)
{
    if (!this->down) this->down = new event<>();
    this->down->add(fn);
}

void clickable::on_released(event<>::callback_t fn)
{
    if (!this->up) this->up = new event<>();
    this->up->add(fn);
}

void draggable::on_ended_drag(event<>::callback_t fn)
{
    if (!this->end_drag) this->start_drag = new event<>();
    this->end_drag->add(fn);
}

void draggable::on_started_drag(event<>::callback_t fn)
{
    if (!this->start_drag) this->start_drag = new event<>();
    this->start_drag->add(fn);
}

focusable::~focusable()
{
    delete this->enter;
    delete this->leave;
    LOGD("%s", __FUNCTION__);
}

clickable::~clickable()
{
    delete this->down;
    delete this->up;
    LOGD("%s", __FUNCTION__);
}

draggable::~draggable()
{
    delete this->start_drag;
    delete this->end_drag;
    LOGD("%s", __FUNCTION__);
}



void clickable::handle_click(unsigned int parent_zindex)
{
    auto mapper = depth_mapper::get_instance();
    auto is_pressed = this->pressed();

    // Sorting clickables by their zindexesw
    if (is_pressed)
    {
        // If current clickable is pressed, check if either is mapper's local clickable is null 
        // or if is current clickable zindex is equal or greater than mapper's clickable's
        // and if is clickable hasn't yet called down event, overwrite mapper's clickable with current.
        auto cur_zindex = parent_zindex + this->zindex;
        if (mapper->__clickable == nullptr || (cur_zindex >= mapper->__clickable->zindex && !mapper->__clickable->__down_called)) 
        {
            mapper->__clickable = this;
            mapper->__clickable_zindex = cur_zindex; 
        }
    }
    else
    {
        // Otherwise if the clickable is not pressed but mapper's clickable is the clickable,
        // clear mapper's clickable 
        if (this == mapper->__clickable)
        {
            mapper->__clickable->__down_called = false;
            if (mapper->__clickable->up) mapper->__clickable->up->call();
            mapper->__clickable = nullptr;
            mapper->__clickable_zindex = 0;
        }
    }
}

bool clickable::pressed()
{
    static auto mapper = depth_mapper::get_instance();
    bool focused = compute_rect().Contains(ImGui::GetMousePos());
    bool pressed = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool holding = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool result = (pressed && focused) || (holding && this->__down_called);
    return result;
}

bool draggable::pressed()
{
    static auto mapper = depth_mapper::get_instance();
    bool focused = drag_canvas->compute_rect().Contains(ImGui::GetMousePos());
    bool pressed = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool holding = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool result = (pressed && focused) || (holding && this->__down_called);
    return result;
}

void draggable::handle_dragging()
{
    auto mapper = depth_mapper::get_instance();

    if (this->pressed() && mapper->__clickable == this && this->__down_called)
    {           
        if (!__start_called && start_drag)
        {
            __start_called = true;
            start_drag->call();
        }
        if (__click_captured)
        {
            pos_px += ImGui::GetMousePos() - __click_pos;
            __recomputing = true;
        }
        __click_pos = ImGui::GetMousePos();
        __click_captured = true;
    }
    else
    {
        if (__click_captured && end_drag) end_drag->call();
        __click_pos = {0, 0};
        __click_captured = false;
    }
}


void rect_draggable::handle_dragging()
{
    auto mapper = depth_mapper::get_instance();

    if (this->pressed() && mapper->__clickable == this && this->__down_called)
    {           
        if (!__start_called && start_drag)
        {
            __start_called = true;
            start_drag->call();
        }
        if (__click_captured)
        {
            auto delta = ImGui::GetMousePos() - __click_pos;

            if (drag_bounds.Min == ImVec2(0, 0) && drag_bounds.Max == ImVec2(0, 0))
            {
                pos_px += delta;
                __recomputing = true;
            }
            else
            {
                auto saved_delta = delta;

                pos_px += delta;
                __recomputing = true;
                auto &rect = compute_rect();

                if (rect.Min.x < drag_bounds.Min.x)
                    delta.x -= (rect.Min.x - drag_bounds.Min.x);
                else if (rect.Max.x > drag_bounds.Max.x)
                    delta.x -= (rect.Max.x - drag_bounds.Max.x);

                if (rect.Min.y < drag_bounds.Min.y)
                    delta.y -= (rect.Min.y - drag_bounds.Min.y);
                else if (rect.Max.y > drag_bounds.Max.y)
                    delta.y -= (rect.Max.y - drag_bounds.Max.y);

                if (delta != saved_delta)
                {
                    pos_px -= saved_delta;
                    pos_px += delta;
                    __recomputing = true;
                }
            }
        }
        __click_pos = ImGui::GetMousePos();
        __click_captured = true;
    }
    else
    {
        if (__click_captured && end_drag) end_drag->call();
        __click_pos = {0, 0};
        __click_captured = false;
    }
}