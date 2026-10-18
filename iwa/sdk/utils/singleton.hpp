#pragma once

namespace Iwa
{
    template <typename T>
    class Singleton
    {
    protected:
        Singleton() noexcept = default;
        virtual ~Singleton() = default;

    public:
        constexpr static T* getInstance() noexcept
        {
            static T* instance = nullptr;

            if (instance == nullptr)
                instance = new T();

            return instance;
        }
    };

    template <typename T>
    class CSingleton
    {
    private:
        static constexpr T __instance = T();
    protected:
        constexpr CSingleton() noexcept = default;
        constexpr ~CSingleton() = default;

    public:
        constexpr static T& getInstance() noexcept
        {
            return __instance;
        }
    };
}
