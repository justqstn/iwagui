#pragma once
#include <vector>
#include <functional>

namespace iwa
{
    template<class ...params_t>
    // event listener
    class event
    {
    public:
        event() = default;

        using callback_t = void(*)(params_t...);

        // for lambdas with captures
        using cptcallback_t = std::function<void(params_t...)>;

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

        void add(callback_t function)
        {
            this->functions.emplace_back(function);
        }

        // for lambdas with captures
        void addcpt(cptcallback_t function)
        {
            this->cptfunctions.emplace_back(function);
        }

        void remove(callback_t function)
        {
            std::erase_if(this->functions, [function](callback_t function2) {
                return function == function2;
            });
        }
    private:
        std::vector<callback_t> functions;
        std::vector<cptcallback_t> cptfunctions;
    };
}