#pragma once
#include <mutex>
#include <shared_mutex>
#include <iostream>
#include <stacktrace>

namespace cisim
{
    template <typename Lock, typename Mutex>
    class log_lock : public Lock<Mutex>
    {
        std::thread::id _id = std::this_thread::get_id();
    public:
        template <typename... Args>
        log_lock(Mutex& mutex, Args&&... args)
            : Lock<Mutex>{ mutex, std::forward<Args>(args)... }
        {
            std::cout << "Locked by: " << _id << std::endl;
            std::cout << std::stacktrace::current() << std::endl;
        }

        ~log_lock()
        {
            std::cout << "Unlocked by: " << _id << std::endl;
        }
    };

    template <typename Mutex>
    using log_unique_lock = log_lock<std::unique_lock, Mutex>;

    template <typename Mutex>
    using log_shared_lock = log_lock<std::shared_lock, Mutex>;

    template <typename Mutex>
    using log_lock_guard = log_lock<std::lock_guard, Mutex>;
}