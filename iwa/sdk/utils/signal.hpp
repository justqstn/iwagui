#pragma once
#include <functional>
#include <vector>

namespace Iwa
{
    /**
     * It's used to simply handle signals via callbacks
     * @tparam params_t Set of Signal arguments
     */

    template <typename... params_t>
    class Signal
    {
    public:
        Signal() = default;

        using callback_t = std::function_ref<void(params_t...)>;

        /**
         * Calls all attached callbacks
         * @param params Signal arguments
         */
        void call(params_t... params)
        {
            for (auto it : this->__functions)
            {
                it(params...);
            }
        }

        /**
         * Attaches callback
         * @param params Callback (lambda, reference to function)
         */
        Iwa::Signal<params_t...>* add(callback_t function)
        {
            this->__functions.emplace_back(function);
            return this;
        }

        /**
         * Removes callback
         * @param params Callback (lambda, reference to function)
         */
        void remove(callback_t function)
        {
            std::erase_if(this->__functions, [function](callback_t function2) { return function == function2; });
        }

    private:
        // @todo: here is bad memory using
        std::vector<callback_t> __functions;
    };
} // namespace Iwa
