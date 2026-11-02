#pragma once

#include "utils/logger.hpp"
#include "utils/macros.hpp"
#include <concepts>
#include <cstdint>
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
                __instances[id] = static_cast<TObject*>(this);
            }
            ~HasInstanceMap() noexcept
            {
                __instances.erase(__id);
            }

            constexpr auto operator<=>(const HasInstanceMap<TObject>&) const = default;

            const std::unordered_map<std::string, TObject*>& getInstances() { return this->__instances; }

            static TObject* getByID(const std::string& id) noexcept
            {
                if (__instances.contains(id))
                    return __instances[id];
                else
                    return nullptr;
            }

            static void Clear() noexcept
            {
                std::vector<TObject*> to_delete;
                to_delete.resize(__instances.size());
                for (auto& it : __instances) to_delete.emplace_back(it.second);
                for (auto it : to_delete) it->Destroy();
                __instances.clear();
            }

            const std::string& getID() const { return __id; }

        private:
            std::string __id;
            static std::unordered_map<std::string, TObject*> __instances;
        };

        template<typename TObject>
        std::unordered_map<std::string, TObject*> HasInstanceMap<TObject>::__instances = {};
    }
} // namespace Iwa
