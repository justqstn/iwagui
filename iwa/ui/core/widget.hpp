#pragma once

#include "imgui.h"
#include "utils.hpp"
#include "event.hpp"
#include "logger.hpp"
#include "canvas.hpp"
#include "singleton.hpp"

#include <optional>
#include <concepts>
#include <set>
#include <algorithm>

namespace iwa
{
    struct abstract_widget_params;
    template <class params_t=abstract_widget_params>
    class abstract_widget;
    using base_widget = abstract_widget<>;

    struct abstract_widget_params : virtual abstract_canvas
    {
    public:
        base_widget* widget = nullptr;
        using widgets_set = std::set<std::string>;
        ImU32 color = 0;
        iwa::event<float>* pre = nullptr;  // Called before rendering. @param float Delta time.
        iwa::event<float>* post = nullptr; // Called after rendering. @param float Delta time.
        widgets_set* parents = nullptr;   
        widgets_set* descendants = nullptr;
        bool enabled = true;

        inline void enable() { this->enabled = true; }
        inline void disable() { this->enabled = false; }
        inline void toggle() { this->enabled = !this->enabled; }

        void add_descendant(const std::string &id);
        void add_parent(const std::string &id);
        void on_pre_draw(iwa::event<float>::callback_t fn);
        void on_post_draw(iwa::event<float>::callback_t fn);
        void destroy_widget();
    };



    class widget_manager : public singleton<widget_manager>
    {
    public:
        widget_manager();   
        void add(const std::string& id, base_widget* widget);
        void remove(base_widget* widget);
        void remove(const std::string& id);
        size_t count();
        base_widget* get(const std::string& id);
    };
    
    template <class params_t>
    class abstract_widget
    {
        static_assert(std::derived_from<params_t, abstract_widget_params>, "Template argument must be a derived from iwa::abstract_widget_params");
    public:
        friend class depth_mapper;
        friend class widget_manager;

        abstract_widget(params_t* data) : __data(data)
        {
            __data->widget = reinterpret_cast<base_widget*>(this);
            this->data()->apply_styles(this);

            if (__data->id == "") __data->id = iwa::random_id();

            widget_manager::get_instance()->add(__data->id, reinterpret_cast<base_widget*>(this));

            if (__data->parents)
            {
                for (auto &it : *__data->parents)
                {
                    auto widget = widget_manager::get_instance()->get(it);
                    widget->data()->add_descendant(__data->id);
                }
            }

            LOGD("Added new widget with id %s.", __data->id.data());
        }

        ~abstract_widget() 
        { 
            LOGD("Removed widget with id %s.", __data->id.data());

            delete __data->parents;
            delete __data->descendants;
            delete __data->pre;
            delete __data->post;
            delete __data; 
        }

        void destroy()
        {
            __destroyed = true;
            __data->enabled = false;

            if (__data->parents)
            {
                for (auto &parent_id : *__data->parents)
                {
                    auto widget = widget_manager::get_instance()->get(parent_id);

                    widget->data()->descendants->erase(__data->id);
                }
                __data->parents->clear();
            }

            

            if (__data->descendants)
            {
                for (auto &descendant_id : *__data->descendants)
                {
                    auto widget = widget_manager::get_instance()->get(descendant_id);
                    widget->destroy();
                }
                 __data->descendants->clear();
            }

           

            widget_manager::get_instance()->remove(__data->id);
            delete this;
        }

        bool alive()
        {
            return !__destroyed;
        }

        inline params_t* data() { return static_cast<params_t*>(this->__data); }

        virtual void render(unsigned int parent_zindex) = 0;
        virtual void draw(float dt) = 0;
    protected:
        virtual void push_to_depth_map(unsigned int parent_zindex) = 0;
        
        virtual void render_descendants(unsigned int parent_zindex)
        {
            if (!__data->enabled || !__data->descendants) return;

            for (auto& descendant_id : *__data->descendants)
            {
                auto widget = widget_manager::get_instance()->get(descendant_id);
                widget->data()->set_bounds(__data->compute_rect());
                if (widget->data()->enabled) widget->render(__data->zindex + parent_zindex);
            }
        }

        virtual void push(unsigned int parent_zindex)
        {
            if (!__data->enabled) return;

            push_to_depth_map(parent_zindex);
            render_descendants(parent_zindex);
        }
    private:
        abstract_widget_params *__data = nullptr;
        bool __destroyed = false;
    };
}