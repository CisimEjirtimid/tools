#pragma once
#include <memory>
#include "count_condition.h"

namespace mdsp
{
    struct Awaitable
    {
    public:
        using Result = CountCondition::Result;
        static constexpr auto OK = CountCondition::OK;
        static constexpr auto Timeout = CountCondition::Timeout;
        static constexpr auto Shutdown = CountCondition::Shutdown;

    protected:
        std::shared_ptr<CountCondition> done = nullptr;

    public:
        Awaitable() = default;

        Awaitable(size_t n)
            : done(std::make_shared<CountCondition>())
        {
            done->expect(n);
        }

        Awaitable(std::shared_ptr<CountCondition> awaiter)
            : done(std::move(awaiter))
        {
        }

        ~Awaitable()
        {
            if (done.use_count() <= 2)
                unblock();
        }

        Awaitable(const Awaitable&) = default;

        Awaitable(Awaitable&& other) noexcept
            : done(std::move(other.done))
        {
            other.done = nullptr;
        }

        Awaitable& operator=(const Awaitable& other)
        {
            unblock();
            done = other.done;

            return *this;
        }

        Awaitable& operator=(Awaitable&& other) noexcept
        {
            unblock();

            done = std::move(other.done);
            other.done = nullptr;

            return *this;
        }

        bool awaitable()
        {
            return bool(done);
        }

        CountCondition::Result wait(std::optional<Time> timeout = {})
        {
            if (done)
                return done->wait(timeout);

            return CountCondition::Shutdown;
        }

        void notify()
        {
            if (done)
            {
                done->notify();
                done = nullptr;
            }
        }

        void unblock()
        {
            if (done)
            {
                done->disable();
                done = nullptr;
            }
        }

        Awaitable forward()
        {
            Awaitable forwarded{ std::move(done) };
            done = nullptr;

            return forwarded;
        }

        static Awaitable Empty()
        {
            return Awaitable();
        }

        static Awaitable Conditional(bool condition, size_t n)
        {
            if (condition)
                return Awaitable(n);

            return Awaitable();
        }
    };

    template<typename T>
    constexpr bool is_awaitable = std::is_base_of_v<Awaitable, T>;
}
