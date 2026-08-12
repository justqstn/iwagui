#pragma once

#include "imgui_internal.h"

#include <string>
#include <functional>
#include <unordered_set>

#include "typedefs.hpp"
#include "attributes.hpp"
#include "macroses.hpp"
#include "canvas.hpp"
#include "utils.hpp"
#include "widget_controller.hpp"
#include "render_controller.hpp"
#include "input_controller.hpp"
#include "context.hpp"

namespace iwa
{
    class context;
    template <class widget_t>
    class abstract_widget : public virtual abstract_canvas, renderable, buildable<widget_t>
    {
        COMPONENT(uint, color, color_t(255,255,255))
        SIGNAL_COMPONENT(float, pre_render, nullptr)
        SIGNAL_COMPONENT(float, post_render, nullptr)
        COMPONENT(bool, enabled, true)
        COMPONENT_PTR(iwa::context, ctx, nullptr)
    public:
        friend class widget_controller;
        friend class render_controller;

        template<typename T, typename U>
        friend struct abstract_widget_builder;

        template<typename T>
        friend class abstract_builder;

        static const std::string DEFAULT_TAG(); // will be used later

        void destroy()
        {
            __active = false;

            // __ctx->widgets.erase(this);

            if (__children)
            {
                for (const auto &id : *__children)
                {
                    auto widget = widget_controller::get_instance()->get(id);
                    widget->destroy();
                }
            }

            widget_controller::get_instance()->remove(this);
        }

        virtual bool active()
        {
            return __active && __enabled; 
        }

        void push(uint parent_zindex) override
        {   
            if (!active()) return;

            auto zindex = this->__zindex + this->__ctx->get_zindex() + parent_zindex;
            render_controller::get_instance()->push(reinterpret_cast<widget *>(this));

            if (__children)
            {
                for (auto &widget_id : *__children)
                {
                    auto widget = widget_controller::get_instance()->get(widget_id);
                    widget->set_bounds(compute_rect());
                    widget->push(zindex + 1);
                }
            }

            if (__children_tags)
            {
                for (auto &widget_tag : *__children_tags)
                {
                    auto& tagged_widgets = widget_controller::get_instance()->get_tagged(widget_tag);
                    
                    for (auto& widget_id : tagged_widgets)
                    {
                        auto widget = widget_controller::get_instance()->get(widget_id);

                        widget->set_bounds(compute_rect());                        
                        widget->push(zindex + 1);
                    }
                }
            }
        }

        abstract_widget() {}
        ~abstract_widget()
        {
            delete __children;
            delete __children_tags;
            delete __tags;
        }

        void build() override
        {
            this->buildable<widget_t>::build();

            if (this->__id == "") 
                this->__id = random_id();

            widget_controller::get_instance()->add(reinterpret_cast<widget*>(this));
        } 

        widget_t* add_child(const std::string& id)
        {
            if (!__children) 
                __children = new std::unordered_set<std::string>;
            __children->emplace(id);

            
            return reinterpret_cast<widget_t*>(this);
        }

        widget_t* add_children(std::initializer_list<const std::string&>& ids)
        {
            if (!__children) 
                __children = new std::unordered_set<std::string>;
            __children->emplace(ids);
            
            return reinterpret_cast<widget_t*>(this);
        }

        widget_t* add_child_tag(const std::string& tag)
        {
            if (!__children_tags) 
                __children_tags = new std::unordered_set<std::string>;
            __children_tags->emplace(tag);

            
            return reinterpret_cast<widget_t*>(this);
        }

        widget_t* add_children_tags(std::initializer_list<const std::string&>& tags)
        {
            if (!__children_tags) 
                __children_tags = new std::unordered_set<std::string>;
            __children_tags->emplace(tags);

            
            return reinterpret_cast<widget_t*>(this);
        }

        widget_t* tag(const std::string& tag)
        {
            if (!__tags) 
                __tags = new std::unordered_set<std::string>;
            __tags->emplace(tag);
            
            return reinterpret_cast<widget_t*>(this);
        }

        widget_t* tag(std::initializer_list<const std::string&>& tags)
        {
            if (!__tags) 
                __tags = new std::unordered_set<std::string>;
            __tags->emplace(tags);
            
            return reinterpret_cast<widget_t*>(this);
        }

        const std::unordered_set<std::string>* get_tags()
        {
            return __tags;
        }

    protected:  
        bool __active = true;
        std::unordered_set<std::string>* __children = nullptr;
        std::unordered_set<std::string>* __children_tags = nullptr;
        std::unordered_set<std::string>* __tags = nullptr;
    private:
        bool __binded = false;
    };
  
    template<class widget_t>
    class abstract_interactible_widget : public abstract_widget<widget_t>, virtual interactible
    {
    public:
        void push(uint parent_zindex) override
        {
            if (!this->active()) return;
            
            this->handle_interact();
            this->abstract_widget<widget_t>::push(parent_zindex);
        }
    };

    
    template<class builder_t, class widget_t>
    struct abstract_widget_builder : abstract_builder<widget_t>
    {
        #undef BUILDER_TYPE
        #define BUILDER_TYPE builder_t

        VALUE(std::string, id) 
        VALUE(color_t, color)
        VALUE(uint, zindex)
        RECOMPUTABLE_VALUE(vec2, anchor)
        RECOMPUTABLE_VALUE(rect_t, bounds)
        RECOMPUTABLE_VALUE(vec2, pos)
        RECOMPUTABLE_VALUE(vec2, pos_px)
        SIGNAL_VALUE(float, pre_render)
        SIGNAL_VALUE(float, post_render)

        builder_t* ctx(const std::string& id)
        {
            this->instance->ctx = context::get(id);
            return static_cast<builder_t *>(this);
        }

        builder_t* bind()
        {
            this->__binded = true;
            return static_cast<builder_t *>(this);
        }

        builder_t* unbind()
        {
            this->__binded = false;
            return static_cast<builder_t *>(this);
        }

        builder_t* child(const std::string& id)
        {
            this->instance->add_child(id);
            return static_cast<builder_t *>(this);
        }

        builder_t* children(std::initializer_list<const std::string&>& ids)
        {
            this->instance->add_children(ids);
            return static_cast<builder_t *>(this);
        }

        builder_t* child_tag(const std::string& tag)
        {
            this->instance->add_child_tag(tag);
            return static_cast<builder_t *>(this);
        }

        builder_t* children_tags(std::initializer_list<const std::string&>& tags)
        {
            this->instance->add_children_tags(tags);
            return static_cast<builder_t *>(this);
        }

        builder_t* tag(const std::string& tag)
        {
            this->instance->tag(tag);
            return static_cast<builder_t *>(this);
        }

        builder_t* tag(std::initializer_list<const std::string&>& tags)
        {
            this->instance->tag(tags);
            return static_cast<builder_t *>(this);
        }

        void validate() override
        {
            auto object = reinterpret_cast<abstract_widget<void*>*>(this->instance);
            if (object->__ctx == nullptr)
            {
                auto global_ctx = iwa::context::get_global();
                if (!global_ctx) assert(false && "Context is null");
                else 
                {
                    object->__ctx = global_ctx;  
                    if (__binded) global_ctx->widgets.emplace(reinterpret_cast<widget*>(this->instance));
                }
            }
            else if (__binded) object->__ctx->widgets.emplace(reinterpret_cast<widget*>(this->instance));
        }   

        #undef BUILDER_TYPE
    protected:
        bool __binded;
    };
}