#pragma once
#include <cstddef>
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
        using listener_id_t = size_t;

        /**
         * Calls all attached callbacks
         * @param params Signal arguments
         */
        void call(params_t... params)
        {
            for (auto& entry : __functions)
            {
                entry.fn(params...);
            }
        }

        /**
         * Attaches callback
         * @return this for chaining
         */
        Iwa::Signal<params_t...>* add(callback_t function)
        {
            connect(function);
            return this;
        }

        /**
         * Attaches callback and returns a stable id for later removal
         */
        listener_id_t connect(callback_t function)
        {
            const listener_id_t id = __next_id++;
            __functions.push_back(Entry{id, function});
            return id;
        }

        /**
         * Removes callback previously returned by connect()
         */
        void disconnect(listener_id_t id)
        {
            std::erase_if(__functions, [id](const Entry& entry) { return entry.id == id; });
        }

        /**
         * Removes callback by id (alias of disconnect)
         */
        void remove(listener_id_t id) { disconnect(id); }

        bool empty() const { return __functions.empty(); }
        size_t size() const { return __functions.size(); }

    private:
        struct Entry
        {
            listener_id_t id = 0;
            callback_t fn;
        };

        std::vector<Entry> __functions;
        listener_id_t __next_id = 1;
    };
} // namespace Iwa
