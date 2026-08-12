#pragma once

#include <unordered_map>
#include <vector>

#include "singleton.hpp"
#include "attributes.hpp"
#include "typedefs.hpp"

namespace iwa
{
    template<typename T>
    using map_t = std::unordered_map<uint, std::vector<T*>>;
    class input_controller final : public singleton<input_controller>
    {
    public:
        input_controller();
        void handle_clickable(clickable* object);

        void update();
    private:
        void __click_signal();
        clickable* __clickable;
        clickable* __last_clickable;
    };
}