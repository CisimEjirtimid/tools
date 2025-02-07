#pragma once
#include <chrono>
#include <tuple>
#include <utility>
#include <type_traits>

namespace mdsp::meta
{
    template<typename Metafunction>
    using eval_t = typename Metafunction::type;

    template<typename T>
    using raw = std::decay_t<T>;

    template<typename... Ts>
    struct TypeList {};

    template<typename T>
    struct Identity
    {
        using type = T;
    };

    namespace detail
    {
        template<typename T>
        struct make_const_ref
        {
            using type = std::add_lvalue_reference_t<std::add_const_t<T>>;
        };

        template<typename T>
        struct make_const_ref<T&>
        {
            using type = std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<T>>>;
        };

        template<typename T>
        struct make_const_ref<T&&> : make_const_ref<T&> {};
    }

    template<typename T>
    using MakeConstRef = detail::make_const_ref<T>;

    namespace detail
    {
        template<typename T>
        struct CountOf : std::integral_constant<size_t, 0> {};

        template<template<typename...> typename L, typename... Ts>
        struct CountOf<L<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};
    }

    template<typename T>
    using CountOf = detail::CountOf<T>;

    template<bool Cond, typename Then, typename Else>
    struct If : Identity<Then> {};

    template<typename Then, typename Else>
    struct If<false, Then, Else> : Identity<Else> {};

    template<typename T>
    struct IsDuration : std::false_type {};

    template<typename T, T num, T den>
    struct IsDuration<std::chrono::duration<T, std::ratio<num, den>>> : std::true_type {};

    namespace detail
    {
        template<typename T, typename U>
        using is_constructible_t = decltype(T(std::declval<U>()));
    }

    template<typename T, typename U, typename = void>
    struct IsConstructibleWith : std::false_type {};

    template<typename T, typename U>
    struct IsConstructibleWith<T, U,
        std::void_t<detail::is_constructible_t<T, U>>> : std::true_type {};

    namespace detail
    {
        template<typename T>
        using has_underlying_type_t = decltype(std::declval<eval_t<T>>());
    }

    template<typename T, typename = void>
    struct HasUnderlyingType : std::false_type {};

    template<typename T>
    struct HasUnderlyingType<T,
        std::void_t<detail::has_underlying_type_t<T>>> : std::true_type {};

    namespace detail
    {
        template<typename T, bool>
        struct UnderlyingType : Identity<T> {};

        template<typename T>
        struct UnderlyingType<T, true> : Identity<eval_t<T>> {};
    }

    template<typename T>
    using UnderlyingType = eval_t<detail::UnderlyingType<T, HasUnderlyingType<T>::value>>;

    template<typename T, typename U>
    struct IsSame : std::bool_constant<
        std::is_same<T, U>::value ||
        std::is_same<T, UnderlyingType<U>>::value ||
        std::is_same<UnderlyingType<T>, U>::value
    > {};

    template<typename T, typename U, typename... Ts>
    struct AllSame : std::false_type {};

    template<typename T>
    struct AllSame<T, T> : std::true_type {};

    template<typename T, typename... Ts>
    struct AllSame<T, T, Ts...> : AllSame<T, Ts...> {};

    namespace detail
    {
        template<typename T, typename... Ts>
        struct First : Identity<T> {};

        template<typename T, typename... Ts>
        struct First<std::tuple<T, Ts...>> : Identity<T> {};
    }

    template<typename... T>
    using First = eval_t<detail::First<T...>>;

    namespace detail
    {
        template<typename L, typename T>
        struct contains;

        template<template<typename...> typename L, typename T>
        struct contains<L<>, T> : std::false_type {};

        template<template<typename...> typename L, typename T, typename... Ts>
        struct contains<L<T, Ts...>, T> : std::true_type {};

        template<template<typename...> typename L, typename T, typename V, typename... Ts>
        struct contains<L<V, Ts...>, T> : contains<L<Ts...>, T> {};
    }

    template<typename L, typename T>
    using Contains = detail::contains<L, T>;

    namespace detail
    {
        template<typename F>
        using is_callable_t = decltype(&F::operator());

        struct nonexisting;
        void empty(nonexisting);

        template<typename F, typename = void>
        struct invoke_type_t
        {
            using type = decltype(&empty);
        };

        template<typename F>
        struct invoke_type_t<F, std::void_t<is_callable_t<F>>>
        {
            using type = decltype(&F::operator());
        };

        template<typename F>
        struct callable_traits
        {
            using invoke_type = typename invoke_type_t<F>::type;
            using underlying_traits = callable_traits<invoke_type>;
            using return_type = typename underlying_traits::return_type;
            using args_list = typename underlying_traits::args_list;

            static constexpr size_t arity = underlying_traits::arity;
        };

        template<typename R, typename... Args>
        struct callable_traits<R(Args...)>
        {
            using return_type = R;
            using args_list = std::tuple<Args...>;

            static constexpr size_t arity = sizeof...(Args);
        };

        template<typename R, typename... Args>
        struct callable_traits<R(*)(Args...)> : callable_traits<R(Args...)> {};

        template<typename C, typename R, typename... Args>
        struct callable_traits<R(C::*)(Args...)> : callable_traits<R(Args...)> {};

        template<typename C, typename R, typename... Args>
        struct callable_traits<R(C::*)(Args...) const> : callable_traits<R(Args...)> {};

        // static_assert has to be used because of C++/CLI
        template<typename Traits, size_t N>
        struct Arg
        {
            static_assert(N < Traits::arity,
                "Number specified as template argument is greater than the number of tuple elements provided");

            using args_list = typename Traits::args_list;

            using type = typename std::tuple_element<N, args_list>::type;
        };
    }

    template<typename F>
    using CallableTraits = detail::callable_traits<std::decay_t<F>>;

    template<typename Traits>
    constexpr size_t Arity = Traits::arity;

    template<typename F>
    constexpr size_t ArityF = Arity<CallableTraits<F>>;

    template<typename Traits>
    constexpr size_t Args = Traits::arity;

    template<typename F>
    constexpr size_t ArgsF = Args<CallableTraits<F>>;

    template<typename Traits>
    constexpr size_t ArgCount = Traits::arity;

    template<typename F>
    constexpr size_t ArgCountF = ArgCount<CallableTraits<F>>;

    template<typename Traits>
    using ArgList = typename Traits::args_list;

    template<typename F>
    using ArgListF = ArgList<CallableTraits<F>>;

    template<typename Traits, size_t N>
    using Arg = typename detail::Arg<Traits, N>::type;

    template<typename F, size_t N>
    using ArgF = Arg<CallableTraits<F>, N>;

    template<typename Traits>
    using ReturnType = typename Traits::return_type;

    template<typename F>
    using ReturnTypeF = ReturnType<CallableTraits<F>>;

    template<typename T, template<typename> typename Concept, typename = void>
    struct ConformsTo : std::false_type {};

    template<typename T, template<typename> typename Concept>
    struct ConformsTo<T, Concept, std::void_t<Concept<T>>> : std::true_type {};

    namespace detail
    {
        template<typename T, template<typename...> typename F>
        struct transform_impl;

        template<template<typename...> typename T, typename... Ts, template<typename...> typename F>
        struct transform_impl<T<Ts...>, F>
        {
            using type = T<eval_t<F<Ts>>...>;
        };
    }

    template<typename T, template<typename...> typename F>
    using Transform = eval_t<detail::transform_impl<T, F>>;
}
