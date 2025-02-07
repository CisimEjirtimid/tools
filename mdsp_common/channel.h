#pragma once
#include <type_traits>
#include "timestamp.h"
#include "sync_queue.h"
#include "awaitable.h"

namespace mdsp
{
    struct ChannelConfig
    {
        Time producerTimeout = 5_s;
        Time consumerTimeout = 5_s;
        size_t capacity = 10ull;

        [[nodiscard]]
        constexpr ChannelConfig withSendTimeout(Time timeout)
        {
            return ChannelConfig{ timeout, consumerTimeout, capacity };
        }

        [[nodiscard]]
        constexpr ChannelConfig withRecvTimeout(Time timeout)
        {
            return ChannelConfig{ producerTimeout, timeout, capacity };
        }

        [[nodiscard]]
        constexpr ChannelConfig withCapacity(size_t cap)
        {
            return ChannelConfig{ producerTimeout, consumerTimeout, cap };
        }
    };

    namespace dispatch
    {
        struct Serial
        {
            template<typename T, typename U>
            void operator()(SyncQueue<T>& q, U&& req)
            {
                q.add(std::forward<U>(req));
            }
        };

        struct Priority
        {
            template<typename T, typename U>
            void operator()(SyncQueue<T>& q, U&& req)
            {
                q.addFront(std::forward<U>(req));
            }
        };
    };

    template<typename Messages>
    struct Channel
    {
        using type = Messages;

        SyncQueue<Messages> q;

        auto recv()
        {
            return q.getWithStatus();
        }

        bool empty()
        {
            return q.isEmpty();
        }

        bool isFull()
        {
            return q.isFull();
        }

        template<typename Dispatch = dispatch::Serial, typename T>
        void send(T&& msg)
        {
            Dispatch{}(q, std::forward<T>(msg));
        }

        void open(const ChannelConfig& config)
        {
            q.producerTimeout(config.producerTimeout);
            q.consumerTimeout(config.consumerTimeout);
            q.capacity(config.capacity);
            q.shouldReceive(true);
        }

        void close()
        {
            q.lock()
                .deny()
                .clear(true)
                .notifyAll();
        }

        template<typename T, typename F>
        void select(F&& handler)
        {
            q.template forEach<std::decay_t<T>>(std::forward<F>(handler));
        }

        template<typename Dispatch = dispatch::Serial, typename T>
        void update(T&& value)
        {
            bool updated = false;

            select<std::decay_t<T>>([&](auto&& existing) {
                if constexpr (is_awaitable<std::decay_t<T>>)
                    existing.unblock();

                existing = std::forward<T>(value);
                updated = true;
            });

            if (!updated)
                send<Dispatch>(std::forward<T>(value));
        }

        void clear()
        {
            q.clear();
        }

        template<typename... Ts>
        void remove()
        {
            q.template remove<Ts...>();
        }
    };

    template<typename Commands>
    using RefChannel = Channel<Commands>&;

    template<typename Commands>
    using SharedChannel = std::shared_ptr<Channel<Commands>>;
}
