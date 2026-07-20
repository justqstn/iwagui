#pragma once

namespace iwa
{
    template <typename T>
    class singleton
    {
    protected:
        singleton() noexcept = default;


        virtual ~singleton() = default;

    public:
        static T* get_instance() noexcept
        {
            static T* instance = nullptr;

            if (instance == nullptr) 
            {
                instance = new T();
            }
            
            return instance;
        }
    };
}