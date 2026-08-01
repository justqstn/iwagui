#pragma once 

#include "singleton.hpp"
#include "widget.hpp"
#include "attributes.hpp"
#include "config.hpp"

#include <map>
#include <vector>
#include <string>

namespace iwa
{
    /**
     * Depth mapper class
     *  - Singleton
     *  - The main concept is every widget pushes theirselfs and their descendants into
     *  one map that sorts by z-indexes. Z-index is like a third coordinate for widgets,
     *  and it's a arbitary number + parent's zindex. As every widget has opportunity to have
     *  more than one parent, you should directly pass parent's zindex every time you render
     *  widget.
     *  - Also this class handles clickables.
     */

    class depth_mapper final : public singleton<depth_mapper>
    {
    public:
        friend class clickable;
        friend class draggable;
        friend class rect_draggable;

        depth_mapper();
        void input();           // Handles inputs from widgets
        void render(float dt);      // Render all widgets that has been added via depth_mapper::push() in order of Z-index
        void push(base_widget* widget, unsigned int parent_zindex); // Pushes into the map
        void push_clip(const ImRect& clip);
        void pop_clip();
        #ifdef IWA_DEPTH_MAPPER_DEBUG
        const std::string& debug_information();
        #endif
    private:
        std::map<unsigned int, std::vector<base_widget*>> __mapped_widgets;
        std::map<unsigned int, std::pair<unsigned int, ImRect>> __clip_rects;
        clickable* __clickable = nullptr;   // Clickable entity
        unsigned int __clickable_zindex = 0;    // Z-index + clickable enity's Z-index
        ImRect __clip_rect_current = {{0,0},{0,0}}; 
        unsigned int __clip_rect_current_id = 0;
        unsigned int __widgets_count = 0;   

        #ifdef IWA_DEPTH_MAPPER_DEBUG
        std::string __debug = "None";       // Debug string that contains some information about mapper. (useless in your builds)
        #endif
    };
}