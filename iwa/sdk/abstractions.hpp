#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include "sdk/utils/logger.hpp"
namespace Iwa
{
    struct IRenderable
    {
        virtual void Render() = 0;
        virtual void Cleanup() = 0;
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
        static TEntity* getByID(const std::string& id) noexcept
        {
            if (__entities.contains(id))
                return __entities[id];
            else
                return nullptr;
        }
        const std::string& getID() { return this->__id; }
    protected:
        std::string __id;
    private:
        static std::unordered_map<std::string_view, TEntity*> __entities;
    };

    template<typename TEntity>
    std::unordered_map<std::string_view, TEntity*> Iwa::AIDEntity<TEntity>::__entities = {};
} // namespace Iwa
