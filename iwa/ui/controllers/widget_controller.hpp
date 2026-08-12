#pragma once

#include <unordered_map>
#include <unordered_set>
#include "singleton.hpp"


namespace iwa
{
    template<class widget_t>
    class abstract_widget;
    using widget = abstract_widget<void>;
    class widget_controller final : public singleton<widget_controller>
    {
    public:
        widget_controller();
        void add(widget* widget);
        void remove(widget* widget);
        void remove(const std::string &id);
        widget* get(const std::string &id);
        const std::unordered_set<std::string>& get_tagged(const std::string& tag);

    private:
        std::unordered_map<std::string, widget*> __widgets;
        std::unordered_map<std::string, std::unordered_set<std::string>> __widget_tags;
    };
}