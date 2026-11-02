#pragma once

#include "utils/macros.hpp"
#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

namespace Iwa
{
    namespace __private__
    {
        template<typename TObject, typename... TConstructorArgs>
        struct HasAllocWrapper
        {
            struct Wrapper : public TObject
            {
                    constexpr IWA_FORCEINLINE Wrapper(TConstructorArgs... args) : TObject(args...) {}
                constexpr IWA_FORCEINLINE ~Wrapper() { }
            };

            constexpr static IWA_FORCEINLINE TObject* New(TConstructorArgs... args)
            {
                return static_cast<TObject*>(new Wrapper(args...));
            }
            constexpr IWA_FORCEINLINE void Destroy() { return delete (static_cast<Wrapper*>(this)); }
        };

        struct AbstractRenderable
        {
            virtual void Render() = 0;
        };

        template<typename TRenderer>
        struct HasRenderer
        {
            HasRenderer(const TRenderer* renderer) : renderer(renderer) {}
            const TRenderer* renderer;
        };

        template <typename TResource>
        struct HasResource
        {
        public:
            constexpr TResource* getResource() const { return __resource; }
            constexpr const bool isLoaded() const { return __resource != nullptr; }
        protected:
            TResource* __resource = nullptr;
        };

        template <typename TObject>
        struct HasInstanceMap
        {
        public:
            HasInstanceMap(const std::string& id)
            {
                __id = id;
                __entities[id] = static_cast<TObject*>(this);
            }
            ~HasInstanceMap() noexcept
            {
                __entities.erase(__id);
            }


            constexpr auto operator<=>(const HasInstanceMap<TObject>&) const = default;

            static TObject* getByID(const std::string& id) noexcept
            {
                if (__entities.contains(id))
                    return __entities[id];
                else
                    return nullptr;
            }
            const std::string& getID() const { return __id; }
            static std::unordered_map<std::string, TObject*> __entities;
        private:
            std::string __id;

        };

        template<typename TObject>
        std::unordered_map<std::string, TObject*> HasInstanceMap<TObject>::__entities = {};
    }
} // namespace Iwa
