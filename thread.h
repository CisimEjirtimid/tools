#pragma once
#include <thread>
#include "mdsp_common/channel.h"

namespace cisim
{
    template <typename State>
    struct DefaultHandlers
    {
        void tick(State& state) {};

        // called outside of started thread
        void onStart(State& state) {}
        void onStop() {} // deinit?

        // called on started thread
        void onEnter(State& state) {}
        void onExit(State& state) {}
    };

    template <typename State, typename Commands>
    class Thread
        : public DefaultHandlers<State>
    {
    protected:
        Channel<Commands> channel;
        std::thread thread;

    public:
        std::atomic_bool running = false;

        Thread()
        {}

        virtual ~Thread()
        {}

        template <typename Self>
        void start(this Self&& self, State state, const ChannelConfig& config)
        {
            if (self.running)
                return;

            self.channel.open(config);

            self.running = true;

            self.onStart(state);

            self.thread = std::thread([&, state = std::move(state)]() mutable
            {
                self.onEnter(state);

                while (self.running)
                {
                    auto [status, cmd] = self.channel.recv();

                    if (status == SyncQStatus::Shutdown)
                        return;

                    std::visit([&](auto&& command) mutable {

                        using C = std::decay_t<decltype(command)>;

                        self.execute(state, command);

                        if constexpr (cisim::is_awaitable<C>)
                            command.notify();
                    }, cmd);

                    self.tick(state);
                }

                self.onExit(state);
            });
        }

        template <typename Self>
        void stop(this Self&& self)
        {
            running = false;

            if (self.thread.joinable())
                self.thread.join();

            self.onStop();
        }



        template<typename Dispatch = dispatch::Serial, typename T>
        void async(T cmd)
        {
            channel.template send<Dispatch>(std::move(cmd));
        }

        template<typename T, typename Dispatch = dispatch::Serial, typename... Ts>
        Awaitable async(Ts&&... args)
        {
            static_assert(cisim::is_awaitable<T>,
                "T is not awaitable and can't be used with async<T>(Ts...) overload"
                );

            Awaitable awaitable(1);

            channel.template send<Dispatch>(T{ awaitable, std::forward<Ts>(args)... });

            return awaitable;
        }
    };
}
