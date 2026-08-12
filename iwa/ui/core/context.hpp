#pragma once

#include "typedefs.hpp"
#include "widget.hpp"
#include "attributes.hpp"
#include "macroses.hpp"

namespace iwa
{
    class context;
    using ctx_map_t = std::unordered_map<std::string, context*>;
    class context final : buildable<context>
    {
        friend class render_controller;

        FRIEND_BUILDER();

        TOGGEABLE_COMPONENT(enabled, true)
        COMPONENT(uint, zindex, 0)
        NCSTREF_COMPONENT(float, alpha, 1.0f)
        CONST_COMPONENT(std::string, id, "")
        NCSTREF_COMPONENT(rect_t, bounds, rect_t()) 
        
        struct builder : abstract_builder<context>, stylable<builder>
        {
            friend context;
            #undef BUILDER_TYPE
            #define BUILDER_TYPE builder

            VALUE(uint, zindex)
            VALUE(float, alpha)
            VALUE(std::string, id)
            VALUE(rect_t, bounds)

            builder* global()
            {
                if (!context::__global_ctx)
                {
                    context::__global_ctx = this->instance;
                }
                else LOGW("Global context is already created.");
                return this;
            }

            void validate() override
            {
                if (this->instance->__bounds.GetArea() == 0) this->instance->__bounds = {{0,0}, get_screen_resolution()};
                if (this->instance->__id == "") this->instance->__id = random_id();
            }

            #undef BUILDER_TYPE
        };
        friend builder;

    public: 
        context();
        ~context();

        std::unordered_set<widget*> widgets = {};
        
        static const ctx_map_t& get_ctx_map();
        static context* get(const std::string& id); 
        static context* get_global();
        void build() override;
        void push();


    private:
        static ctx_map_t __ctxs;    
        static context* __global_ctx;
    };
}