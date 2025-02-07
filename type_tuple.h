#pragma once
#include <tuple>

namespace cisim
{
    template<class... Types>
    struct TypeTuple
    {
        using asTuple = std::tuple<Types...>;

        template<std::size_t I>
        using get = std::tuple_element_t<I, asTuple>;

        static constexpr std::size_t size = sizeof...(Types);
    };
}
