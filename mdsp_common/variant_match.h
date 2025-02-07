#pragma once
#include "meta.h"
#include <variant>

namespace mdsp::match
{
    namespace detail
    {
        template<typename... Ts>
        struct match_exists;

        template<typename... Fs, typename... Vs>
        struct match_exists<std::tuple<Fs...>, std::tuple<Vs...>>
            : std::bool_constant<(std::is_invocable_v<Fs, Vs...> || ...)> {};

        template<typename F, typename... Vs>
        struct arity_matches
            : std::bool_constant<meta::CallableTraits<F>::arity == sizeof...(Vs)> {};

        template<typename... Ts>
        struct valid_arity;

        template<typename... Fs, typename... Vs>
        struct valid_arity<std::tuple<Fs...>, std::tuple<Vs...>>
            : std::bool_constant<(arity_matches<Fs, Vs...>::value && ...)> {};

        template<typename... Ts> struct overloaded : Ts... { using Ts::operator()...; };
        template<typename... Ts> overloaded(Ts...)->overloaded<Ts...>;
    }

    template<typename T>
    auto assign_to(T& value)
    {
        return [&](std::decay_t<T> unwrapped) {
            value = unwrapped;
        };
    }

    template<typename T>
    auto move_in(T& value)
    {
        return [&](std::decay_t<T> unwrapped) {
            value = std::move(unwrapped);
        };
    }

    inline auto rethrow()
    {
        return [](std::runtime_error e) {
            throw e;
        };
    }

    constexpr auto discard = [](auto&&...) {};

    template<typename... Ts>
    auto exhaustive(Ts&&... vars)
    {
        return [&](auto&&... matchers) {
            return std::visit(detail::overloaded{ std::forward<decltype(matchers)>(matchers)... },
                std::forward<Ts>(vars)...
            );
        };
    }

    template<typename... Ts>
    auto nonexhaustive(Ts&&... vars)
    {
        return [&](auto&&... matchers) {
            using Fs = std::tuple<decltype(matchers)...>;
            using Vs = std::tuple<Ts...>;

            static_assert(detail::valid_arity<Fs, Vs>(),
                "Number of template arguments specified for the matcher is invalid");

            return match::exhaustive(std::forward<Ts>(vars)...)(
                std::forward<decltype(matchers)>(matchers)...,
                match::discard
            );
        };
    }
}
