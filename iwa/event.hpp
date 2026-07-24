#pragma once
#include <vector>
#include <functional>


namespace iwa
{
    /**
    * @brief It's used to simply handle events via callbacks 
    * @tparam params_t Set of event arguments
    */
    template<class ...params_t>
    class event
    {
    public:
        event() = default;

        using callback_t = void(*)(params_t...);
        using cptcallback_t = std::function<void(params_t...)>;

        /**
         * @brief Calls all attached callbacks
         * @param params Event arguments
         */
        void call(params_t... params) 
        {
            for (auto it : this->functions)
            {
                it(params...);
            }

            for (auto it : this->cptfunctions)
            {
                it(params...);
            }
        }

        
        /**
         * @brief Attaches callback
         * @param params Callback (lambda, reference to function)
         * @warning To work with lambdas captures use event::addcpt
         */
        void add(callback_t function)
        {
            this->functions.emplace_back(function);
        }

        /**
         * @brief Attaches callback
         * @param params Callback (lambda, reference to function)
         * @note Uses std::function instead of raw pointers
         */
        void addcpt(cptcallback_t function)
        {
            this->cptfunctions.emplace_back(function);
        }

        /**
         * @brief Removes callback
         * @param params Callback (lambda, reference to function)
         */
        void remove(callback_t function)
        {
            std::erase_if(this->functions, [function](callback_t function2) {
                return function == function2;
            });
        }
    private:
        // @todo: here is bad memory using
        std::vector<callback_t> functions;
        std::vector<cptcallback_t> cptfunctions;
    };
}