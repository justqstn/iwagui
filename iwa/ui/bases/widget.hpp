#pragma once

#include "imgui.h"
#include "utils.hpp"
#include "event.hpp"
#include "logger.hpp"
#include "canvas.hpp"
#include <vector>

#define ASPECT_RATIO ImVec2(16,10) 

namespace iwa
{
    enum class corners_flags
    {
        none = 0,
        upper_left = 2,
        upper_right = 4,
        lower_right = 8,
        lower_left = 16
    };

    class widget
    {
    public:
        friend class parent_widget;
        friend class zindex_manager;

        struct params
        {
        public:
            friend widget;
            ImU32 color;
            iwa::event<float> pre;  // @brief Called before rendering. @param float Delta time.
            iwa::event<float> post; // @brief Called after rendering. @param float Delta time.
            u_short zindex = 0;
            bool enabled = true;
            void enable();
            void disable();
            void toggle();
        };
        widget();
        virtual void render() = 0;
        virtual canvas& get_canvas() = 0;
        static widget* get(unsigned int id);
        unsigned int get_id();
        
    protected:
        virtual void draw(float dt) = 0;

        bool __id_initialized;
        unsigned int __id;
        u_short __zindex = 0;
        u_short __parent_zindex = 0;
    };    
}