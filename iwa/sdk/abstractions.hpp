#pragma once

#include "sdk/macroses.hpp"
#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

namespace Iwa
{
    namespace Internal
    {
        template<typename TObject, typename... TConstructorArgs>
        struct HasAllocWrapper
        {
            struct Wrapper : public TObject
            {
                constexpr IWA_FORCEINLINE Wrapper(TConstructorArgs... args) : TObject(args...) {}
                constexpr IWA_FORCEINLINE ~Wrapper() { delete this; }
            };

            constexpr static IWA_FORCEINLINE TObject* New(TConstructorArgs... args)
            {
                return static_cast<TObject*>(new Wrapper(args...));
            }
            constexpr IWA_FORCEINLINE void Destroy() { return delete static_cast<Wrapper*>(this); }
        };

        struct IRenderable
        {
            virtual void Render() = 0;
            virtual void Cleanup() = 0;
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
            constexpr TResource* getResource() const { return this->__resource; }
            constexpr const bool isLoaded() const { return this->__resource != nullptr; }
        protected:
            TResource* __resource = nullptr;
        };

        template <typename TEntity>
        struct AIDEntity
        {
        public:
            AIDEntity(const std::string& id)
            {
                this->__id = id;
                __entities[id] = static_cast<TEntity*>(this);
            }
            ~AIDEntity() noexcept
            {
                __entities.erase(this->__id);
            }


            constexpr auto operator<=>(const AIDEntity<TEntity>&) const = default;

            static TEntity* getByID(const std::string& id) noexcept
            {
                if (__entities.contains(id))
                    return __entities[id];
                else
                    return nullptr;
            }
            const std::string& getID() const { return this->__id; }
        private:
            std::string __id;
            static std::unordered_map<std::string_view, TEntity*> __entities;
        };

        template<typename TEntity>
        std::unordered_map<std::string_view, TEntity*> AIDEntity<TEntity>::__entities = {};
    }
} // namespace Iwa
