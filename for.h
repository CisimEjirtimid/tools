#pragma once
#include <type_traits>
#include "tuple.h"

namespace cisim
{
    template <typename Tuple, typename F>
        requires tuple::concepts::tuple_like<Tuple>
    constexpr void for_each(F&& f, Tuple&& t)
    {
        [] <auto... Is> (Tuple && t, F && f, std::index_sequence<Is...>) {
            (f(std::get<Is>(t)), ...);
        }(std::forward<Tuple>(t), std::forward<F>(f),
            std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
    }

    template <typename F, typename... Ts>
    constexpr void for_each(F&& f, Ts&&... ts)
    {
        (f.template operator()<Ts>(ts), ...);
    }

    template <typename... Ts, typename F>
    constexpr void for_types(F&& f)
    {
        (f.template operator()<Ts>(), ...);
    }

    template <auto... Xs, typename F>
    constexpr void for_values(F&& f)
    {
        (f.template operator()<Xs>(), ...);
    }

    template <auto B, auto E, typename F>
    constexpr void for_range(F&& f)
    {
        using T = std::common_type_t<decltype(B), decltype(E)>;

        [&f] <auto... Xs>(std::integer_sequence<T, Xs...>) {
            for_values<(B + Xs)...>(f);
        }(std::make_integer_sequence<T, E - B>{});
    }

    /*
    * Examples:
    * cisim::for_types<int, char, float, std::string, std::vector<std::string>>([] <typename T>() {
    *     std::cout << typeid(T).name() << std::endl;
    * });
    * 
    * cisim::for_values<0, 1, 2, 3>([] <auto X>() {
    *     std::cout << X << std::endl;
    * });
    * 
    * cisim::for_range<0, 4>([] <auto X>() {
    *     std::cout << X << std::endl;
    * });
    * 
    * cisim::C c1{
    *     .val1 = 1,
    *     .val2 = 2
    * };
    * 
    * cisim::C c2{
    *     .val1 = 3,
    *     .val2 = 4
    * };
    * 
    * cisim::for_each([] <typename T>(T&& t) {
    *     cisim::for_range<0, 4>([&t] <auto I>() {
    *         t.val1 *= (1 + I);
    *         t.val2 *= (1 + I);
    * 
    *         t.print();
    *     });
    * }, c1, c2);
    * 
    * cisim::for_each([] <typename T>(T&& t) {
    *     cisim::for_range<0, 4>([&t] <auto I>() {
    *         t.val1 += I;
    *         t.val2 -= I;
    * 
    *         t.print();
    *     });
    * }, std::tie(c1, c2));
    */
}