#pragma once
#include <mutex>
#include <condition_variable>
#include <optional>
#include "timestamp.h"

namespace mdsp
{
    class CountCondition
    {
    protected:
        bool _enabled = false;
        size_t _count = 0;
        size_t _expectedCount = 0;
        std::mutex _mutex;
        std::condition_variable _var;

    public:
        enum class Result
        {
            OK,
            Shutdown,
            Timeout
        };

        static constexpr auto OK = Result::OK;
        static constexpr auto Shutdown = Result::Shutdown;
        static constexpr auto Timeout = Result::Timeout;

        bool disable()
        {
            std::unique_lock lock{ _mutex };

            if (!_enabled)
                return false;

            _enabled = false;
            lock.unlock();

            _var.notify_all();
            return true;
        }

        void expect(size_t expectedCount)
        {
            std::lock_guard lock{ _mutex };

            _enabled = true;
            _count = 0;
            _expectedCount = expectedCount;
        }

        void notify()
        {
            std::unique_lock lock{ _mutex };

            ++_count;
            lock.unlock();

            _var.notify_all();
        }

        Result wait(std::optional<Time> timeout = {})
        {
            auto shouldWait = [&]() {
                return _enabled && _count != _expectedCount;
            };

            std::unique_lock lock{ _mutex };

            if (_expectedCount == 0)
                return Result::OK;

            bool timedout = false;
            auto waitTime = timeout.value_or(Time::FromSeconds(INT_MAX));

            if (shouldWait())
                timedout = !_var.wait_for(lock, waitTime.chronoMilliseconds(), [&]() { return !shouldWait(); });

            if (!_enabled || timedout)
                return !_enabled ? Result::Shutdown : Result::Timeout;

            return Result::OK;
        }
    };
}
