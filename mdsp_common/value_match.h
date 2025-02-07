#pragma once
#include "meta.h"

namespace mdsp::match
{
    // Utility for easier value matching.
    // Instead of if/else-if chains one could simply use match::values.
    // match::values returns a lambda which accepts comma separated pairs of patterns (std::tuple of values) and handlers (functions).
    // Patterns can also contain an ignored element (match::_ / match::ignore).
    //
    // Usage example 1:
    //      double a = 0.0;
    //      double b = 1.0;
    //      char c = 'c';
    //
    //      match::values(a, b, c) (
    //          std::tuple{ 0.0, 0.0, 'a' },                []() { std::cout << "0.0, 0.0, a" << std::endl; },
    //          std::tuple{ 0.0, 0.0, match::_ },           []() { std::cout << "0.0, 0.0, _" << std::endl; },
    //          std::tuple{ match::_, match::_, 'c' },      []() { std::cout << "_, _, c" << std::endl; },
    //          std::tuple{ match::_, match::_, match::_ }, []() { std::cout << "_, _, _" << std::endl; }
    //      );
    //
    // This is equivalent to:
    //      using namespace match::shortened;
    //
    //      match::values(a, b, c) (
    //          vals(0.0, 0.0, 'a'), []() { std::cout << "0.0, 0.0, a" << std::endl; },
    //          vals(0.0, 0.0, _),   []() { std::cout << "0.0, 0.0, _" << std::endl; },
    //          vals(_, _, 'c'),     []() { std::cout << "_, _, c" << std::endl; },
    //          vals(_, _, _),       []() { std::cout << "_, _, _" << std::endl; }
    //      );
    //
    // Usage example 2:
    //      using namespace match::shortened;
    //
    //      double a = 1.0;
    //
    //      char c = match::values(a) (
    //          vals(0.0), []() { return '0'; },
    //          vals(2.0), []() { return '2'; },
    //          vals(1.0), []() { return '1'; },
    //          vals(_),   []() { return '_'; }
    //      );

    namespace detail
    {
        struct ignore_t {};

        template<int... Is>
        using sequence = std::integer_sequence<int, Is...>;

        template<int N>
        using make_sequence = std::make_integer_sequence<int, N>;

        template<int... Is, int... Js>
        constexpr sequence<Is..., Js...> operator+(sequence<Is...>, sequence<Js...>)
        {
            return {};
        }

        template<size_t I, typename Tuple>
        using Element = std::decay_t<std::tuple_element_t<I, Tuple>>;

        template<typename T>
        struct IsHomogeneous;

        template<typename T>
        struct IsHomogeneous<std::tuple<T>> : std::true_type {};

        template<typename... Ts>
        struct IsHomogeneous<std::tuple<Ts...>> : meta::AllSame<std::decay_t<Ts>...> {};

        template<typename T>
        struct IsTuple : std::false_type {};

        template<typename... Ts>
        struct IsTuple<std::tuple<Ts...>> : std::true_type {};

        template<typename T>
        struct AreTuples;

        template<typename... Ts>
        struct AreTuples<std::tuple<Ts...>>
            : std::bool_constant<
                (IsTuple<std::decay_t<Ts>>::value && ...)
            > {};

        template<typename T, typename U>
        struct TupleSizesEqual;

        template<typename... Ts, typename... Us>
        struct TupleSizesEqual<std::tuple<Ts...>, std::tuple<Us...>>
            : std::bool_constant<
                ((std::tuple_size_v<std::tuple<Ts...>> == std::tuple_size_v<Us>) && ...)
            > {};

        template<typename T>
        struct ReturnTypesImpl;

        template<typename... Ts>
        struct ReturnTypesImpl<std::tuple<Ts...>>
        {
            using type = std::tuple<meta::ReturnTypeF<Ts>...>;
        };

        template<typename T>
        using ReturnTypes = meta::eval_t<ReturnTypesImpl<std::decay_t<T>>>;

        template<typename T, typename U>
        struct ContainsOnly;

        template<typename T, typename... Ts>
        struct ContainsOnly<T, std::tuple<Ts...>>
            : std::bool_constant<
                (std::is_same_v<T, std::decay_t<Ts>> && ...)
            > {};

        template<typename T>
        struct HasCatchAll;

        template<typename... Ts>
        struct HasCatchAll<std::tuple<Ts...>>
            : std::bool_constant<
                (ContainsOnly<ignore_t, Ts>::value || ...)
            > {};

        template<int I, typename F>
        constexpr auto filter(F predicate)
        {
            if constexpr (predicate(I))
            {
                return sequence<I>{};
            }
            else
            {
                return sequence<>{};
            }
        }

        template<int... Is, typename F>
        constexpr auto filter(sequence<Is...>, F predicate)
        {
            return (filter<Is>(predicate) + ...);
        }

        template<int I, typename Pattern>
        constexpr auto removeIgnored_impl()
        {
            using T = Element<I, Pattern>;

            if constexpr (!std::is_same_v<T, ignore_t>)
            {
                return sequence<I>{};
            }
            else
            {
                return sequence<>{};
            }
        }

        template<typename Pattern, int... Is>
        constexpr auto removeIgnored_impl(sequence<Is...>)
        {
            return (removeIgnored_impl<Is, Pattern>() + ...);
        }

        template<typename Pattern>
        constexpr auto removeIgnored()
        {
            constexpr int size = std::tuple_size_v<Pattern>;

            return removeIgnored_impl<Pattern>(make_sequence<size>{});
        }

        template<typename... Ts, int... Is>
        constexpr auto extract(const std::tuple<Ts...>& tuple, sequence<Is...>)
        {
            return std::make_tuple(std::get<Is>(tuple)...);
        }

        template<typename... Ts, typename... Vs, int... Is>
        bool eq(const std::tuple<Ts...>& ts, const std::tuple<Vs...>& vs, sequence<Is...>)
        {
            return ((std::get<Is>(ts) == std::get<Is>(vs)) && ...);
        }

        // Never actually executed
        // Exists simply because expressions regarding match_impl need to be well formed in case of an empty index sequence
        template<typename R, typename... Vs, typename... Ps, typename... Fs>
        auto match_impl(const std::tuple<Vs...>& values, const std::tuple<Ps...>& patterns, const std::tuple<Fs...>& fs, sequence<>)
        {
            if constexpr (!std::is_same_v<R, void>)
                return R{};
            else
                return;
        }

        template<typename R, typename... Vs, typename... Ps, typename... Fs, int I, int... Is>
        auto match_impl(const std::tuple<Vs...>& values, const std::tuple<Ps...>& patterns, const std::tuple<Fs...>& fs, sequence<I, Is...> s)
        {
            using Pattern = Element<I, std::tuple<Ps...>>;

            auto indices = removeIgnored<Pattern>();

            if (eq(values, std::get<I>(patterns), indices))
                return std::get<I>(fs)();

            return match_impl<R>(values, patterns, fs, sequence<Is...>{});
        }

        constexpr auto isEven = [](int i) { return i % 2 == 0; };
        constexpr auto isOdd = [](int i) { return i % 2 != 0; };
    }

    static constexpr detail::ignore_t _;
    static constexpr detail::ignore_t ignore;

    template<typename... Values>
        requires (sizeof...(Values) > 0)
    auto values(Values&&... values)
    {
        std::tuple vals{ std::forward<Values>(values)... };

        return [vals = std::move(vals)](auto&&... pairs) {
            constexpr int size = sizeof...(pairs);

            static_assert(size > 0,
                "Match called without patterns and handlers");
            static_assert(size % 2 == 0,
                "Match requires an even number of arguments");

            constexpr auto indices = detail::make_sequence<size>{};
            constexpr auto patternsSeq = filter(indices, detail::isEven);
            constexpr auto handlersSeq = filter(indices, detail::isOdd);

            std::tuple args{ std::forward<decltype(pairs)>(pairs)... };
            std::tuple patterns = detail::extract(args, patternsSeq);
            std::tuple handlers = detail::extract(args, handlersSeq);

            static_assert(detail::AreTuples<decltype(patterns)>(),
                "Matching patterns need to be tuples");

            static_assert(detail::TupleSizesEqual<std::remove_const_t<decltype(vals)>, decltype(patterns)>(),
                "Matching patterns need to have equal number of arguments as the supplied values");

            static_assert(detail::HasCatchAll<decltype(patterns)>(),
                "Match missing a catch all pattern");

            using RetTypes = detail::ReturnTypes<decltype(handlers)>;

            static_assert(detail::IsHomogeneous<RetTypes>(),
                "Match handlers are required to have the same return type");

            using R = meta::First<RetTypes>;

            return detail::match_impl<R>(vals, patterns, handlers, detail::make_sequence<size / 2>{});
        };
    }

    template<typename... Values>
    auto vals(Values&&... values)
    {
        return std::tuple{ std::forward<Values>(values)... };
    }

    template<typename... Values>
    auto case_(Values&&... values)
    {
        return std::tuple{ std::forward<Values>(values)... };
    }

    template<typename... Values>
    auto pattern(Values&&... values)
    {
        return std::tuple{ std::forward<Values>(values)... };
    }

    namespace shortened
    {
        using match::_;
        using match::vals;
    }
}
