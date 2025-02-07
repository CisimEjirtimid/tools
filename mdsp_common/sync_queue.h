#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <variant>
#include <type_traits>
#include <limits>
#include "timestamp.h"
#include "strong_typedef.h"
#include <concepts>

#undef min
#undef max

namespace mdsp
{
    namespace detail
    {
        template<typename T>
        struct IsVariant : std::false_type {};

        template<typename... Ts>
        struct IsVariant<std::variant<Ts...>> : std::true_type {};
    }

    using ClearCache = StrongTypedef<bool, struct _ClearCacheTag>;

    enum class SyncQStatus
    {
        OK = 0,
        Timeout = 1,
        Shutdown = 2
    };

    template<typename T>
    class SyncQueue
    {
    protected:
        friend class Interlocked;

        class Interlocked
        {
        protected:
            SyncQueue<T>& _self;
            std::mutex& _mutex;

        public:
            Interlocked(SyncQueue<T>& self, std::mutex& mutex)
                : _self(self)
                , _mutex(mutex)
            {
                _mutex.lock();
            }

            ~Interlocked()
            {
                _mutex.unlock();
            }

            Interlocked& shouldReceive(bool value)
            {
                _self._shouldReceive = value;

                return *this;
            }

            Interlocked& receive()
            {
                _self._shouldReceive = true;

                return *this;
            }

            Interlocked& deny()
            {
                _self._shouldReceive = false;

                return *this;
            }

            Interlocked& clear(bool shouldClear)
            {
                if (shouldClear)
                    _self._q.clear();

                return *this;
            }

            Interlocked& notifyConsumer()
            {
                _self.notifyConsumer();

                return *this;
            }

            Interlocked& notifyConsumers()
            {
                _self.notifyConsumers();

                return *this;
            }

            Interlocked& notifyProducer()
            {
                _self.notifyProducer();

                return *this;
            }

            Interlocked& notifyProducers()
            {
                _self.notifyProducers();

                return *this;
            }

            Interlocked& notifyAll()
            {
                _self.notifyAll();

                return *this;
            }
        };

        Time _producerTimeout = Time::FromSeconds(std::numeric_limits<int>::max());
        Time _consumerTimeout = Time::FromSeconds(5);
        size_t _capacity;

        std::mutex _mutex;
        std::condition_variable _notFull;
        std::condition_variable _notEmpty;
        bool _shouldReceive;

        std::deque<T> _q;

        template<typename F>
        auto whenEnqueued(F&& handler, Time timeout)
        {
            std::unique_lock lock{ _mutex };

            bool timedout = false;

            if (consumerShouldWait())
                timedout = !_notEmpty.wait_for(lock, timeout.chronoMilliseconds(), [&]() { return !consumerShouldWait(); });

            if (timedout || _isEmpty_impl())
                return;

            handler();
        }

        size_t _size_impl() const
        {
            return _q.size();
        }

        size_t _capacity_impl() const
        {
            return _capacity;
        }

        bool _isEmpty_impl() const
        {
            return _q.empty();
        }

        bool _isFull_impl() const
        {
            if (_capacity == 0)
                return false;

            return _q.size() >= _capacity;
        }

        bool producerShouldWait() const
        {
            return _shouldReceive && _isFull_impl();
        }

        bool consumerShouldWait() const
        {
            return _shouldReceive && _isEmpty_impl();
        }

    public:
        SyncQueue(size_t capacity = 10)
            : _capacity(capacity)
            , _shouldReceive(true)
        {
        }

        size_t size()
        {
            std::unique_lock lock{ _mutex };

            return _size_impl();
        }

        size_t capacity()
        {
            std::unique_lock lock{ _mutex };

            return _capacity_impl();
        }

        void capacity(size_t newCapacity)
        {
            std::unique_lock lock{ _mutex };

            _capacity = newCapacity;
        }

        bool isEmpty()
        {
            std::unique_lock lock{ _mutex };

            return _isEmpty_impl();
        }

        bool isFull()
        {
            std::unique_lock lock{ _mutex };

            return _isFull_impl();
        }

        Time producerTimeout()
        {
            std::unique_lock lock{ _mutex };

            return _producerTimeout;
        }

        void producerTimeout(Time timeout)
        {
            std::unique_lock lock{ _mutex };

            _producerTimeout = timeout;
        }

        Time consumerTimeout()
        {
            std::unique_lock lock{ _mutex };

            return _consumerTimeout;
        }

        void consumerTimeout(Time timeout)
        {
            std::unique_lock lock{ _mutex };

            _consumerTimeout = timeout;
        }

        void notifyProducer()
        {
            _notFull.notify_one();
        }

        void notifyProducers()
        {
            _notFull.notify_all();
        }

        void notifyConsumer()
        {
            _notEmpty.notify_one();
        }

        void notifyConsumers()
        {
            _notEmpty.notify_all();
        }

        void notifyAll()
        {
            _notEmpty.notify_all();
            _notFull.notify_all();
        }

        void clear()
        {
            std::unique_lock lock{ _mutex };

            _q.clear();

            lock.unlock();
            notifyAll();
        }

        template<typename... Ts>
            requires (detail::IsVariant<T>::value && sizeof...(Ts) > 0)
        void remove()
        {
            std::unique_lock lock{ _mutex };

            std::deque<T> copies;

            while (!_isEmpty_impl())
            {
                auto value = std::move(_q.front());
                _q.pop_front();

                if (!(... || std::holds_alternative<Ts>(value)))
                    copies.push_back(std::move(value));
            }

            while (!copies.empty())
            {
                auto value = std::move(copies.front());
                copies.pop_front();

                _q.push_back(std::move(value));
            }

            lock.unlock();
            notifyAll();
        }

        void shouldReceive(bool value, ClearCache shouldClear = ClearCache(true))
        {
            std::unique_lock lock{ _mutex };

            _shouldReceive = value;

            if (shouldClear)
                _q.clear();

            lock.unlock();
            notifyAll();
        }

        bool add(T item)
        {
            std::unique_lock lock{ _mutex };

            if (producerShouldWait())
                _notFull.wait_for(lock, _producerTimeout.chronoMilliseconds(), [&]() { return !producerShouldWait(); });

            if (!_shouldReceive)
                return false;

            _q.push_back(std::move(item));

            lock.unlock();
            notifyConsumer();

            return !_isEmpty_impl();
        }

        bool addFront(T item)
        {
            std::unique_lock lock{ _mutex };

            if (producerShouldWait())
                _notFull.wait_for(lock, _producerTimeout.chronoMilliseconds(), [&]() { return !producerShouldWait(); });

            if (!_shouldReceive)
                return false;

            _q.push_front(std::move(item));

            lock.unlock();
            notifyConsumer();

            return !_isEmpty_impl();
        }

        bool tryAdd(T& item)
        {
            std::unique_lock lock{ _mutex, std::try_to_lock };

            if (!lock || !_shouldReceive || _isFull_impl())
                return false;

            _q.push_back(std::move(item));

            lock.unlock();
            notifyConsumer();

            return !_isEmpty_impl();
        }

        T get()
        {
            std::unique_lock lock{ _mutex };

            bool timedout = false;

            if (consumerShouldWait())
                timedout = !_notEmpty.wait_for(lock, _consumerTimeout.chronoMilliseconds(), [&]() { return !consumerShouldWait(); });

            if (timedout || _isEmpty_impl())
                return {};

            auto item = std::move(_q.front());
            _q.pop_front();

            lock.unlock();
            notifyProducer();

            return item;
        }

        std::pair<SyncQStatus, T> getWithStatus()
        {
            std::unique_lock lock{ _mutex };

            bool timedout = false;

            if (consumerShouldWait())
                timedout = !_notEmpty.wait_for(lock, _consumerTimeout.chronoMilliseconds(), [&]() { return !consumerShouldWait(); });

            if (timedout || _isEmpty_impl())
                return { !_shouldReceive ? SyncQStatus::Shutdown : SyncQStatus::Timeout, T{} };

            auto item = std::move(_q.front());
            _q.pop_front();

            lock.unlock();
            notifyProducer();

            return { SyncQStatus::OK, std::move(item) };
        }

        bool tryGet(T& item)
        {
            std::unique_lock lock{ _mutex, std::try_to_lock };

            if (!lock || _isEmpty_impl())
                return false;

            item = std::move(_q.front());
            _q.pop_front();

            lock.unlock();
            notifyProducer();

            return true;
        }

        template<typename F>
        void peekFront(F&& handler, Time timeout = Time::FromSeconds(5))
        {
            whenEnqueued([&]()
            {
                handler(_q.front());
            }, timeout);
        }

        template<typename F>
        void peekBack(F&& handler, Time timeout = Time::FromSeconds(5))
        {
            whenEnqueued([&]()
            {
                handler(_q.back());
            }, timeout);
        }

        template<typename F>
        void forEach(F&& handler)
        {
            std::unique_lock lock{ _mutex };

            for (auto& item : _q)
                handler(item);
        }

        template<typename U, typename F>
            requires detail::IsVariant<T>::value
        void forEach(F&& handler)
        {
            std::unique_lock lock{ _mutex };

            for (auto& var : _q)
            {
                if (!std::holds_alternative<U>(var))
                    continue;

                std::visit([&](auto& value) {
                    using V = std::decay_t<decltype(value)>;

                    if constexpr (std::is_same<U, V>())
                        handler(value);
                }, var);
            }
        }

        Interlocked lock()
        {
            return Interlocked{ *this, _mutex };
        }
    };

    template<typename... Ts>
    auto forEachQ(SyncQueue<Ts>&... ts)
    {
        return [&](auto&& handler)
        {
            (handler(ts), ...);
        };
    }
}
