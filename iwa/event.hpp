#pragma once
#include <vector>
#include <functional>


namespace iwa
{
    /**
    * It's used to simply handle events via callbacks 
    * @tparam params_t Set of event arguments
    */

    
    template<class ...params_t>
    class event
    {
    public:
        event() = default;

        using callback_t = std::function<void(params_t...)>;

        /**
         * Calls all attached callbacks
         * @param params Event arguments
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
        iwa::event<params_t...>* add(callback_t function)
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
            std::erase_if(this->__functions, [function](callback_t function2) {
                return function == function2;
            });
        }
    private:
        // @todo: here is bad memory using
        std::vector<callback_t> __functions;
    };
}