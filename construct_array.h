#pragma once
#include <array>

namespace cisim
{
    namespace index
    {
        struct pass {};
        struct no_pass {};

        template <typename T>
        concept passing = std::is_same_v<T, pass> || std::is_same_v<T, no_pass>;
    }

    namespace _detail
    {
        template <typename T, index::passing Policy, size_t... Seq, typename... Args>
        constexpr std::array<T, sizeof...(Seq)> construct_array(std::index_sequence<Seq...>, Args&&... args)
        {
            if constexpr (std::is_same_v<Policy, index::pass>)
                return { { (static_cast<void>(Seq), T{ Seq, std::forward<Args>(args)... })... } };
            else
                return{ { (static_cast<void>(Seq), T{ std::forward<Args>(args)... })... } };
        }
    }

    template <typename T, size_t N, index::passing Policy = index::no_pass, typename... Args>
    constexpr std::array<T, N> construct_array(Args&&... args)
    {
        return _detail::construct_array<T, Policy>(std::make_index_sequence<N>(), std::forward<Args>(args)...);
    }
}