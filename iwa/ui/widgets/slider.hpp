#pragma once
#include "button.hpp"
#include <concepts>

template <typename data_t>
concept numeric = std::integral<data_t> || std::floating_point<data_t>;

namespace iwa
{
    template<numeric data_t>
    class slider;
    struct slider_params : abstract_widget_params, focusable
    {
        
    };

    template<numeric data_t>
    class slider : abstract_widget<slider_params>
    {
    public:
    };
}