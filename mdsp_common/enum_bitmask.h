#pragma once
#include <type_traits>
#include <array>

namespace mdsp
{
    template<typename E>
    struct EnumBitmask : std::false_type {};

    namespace detail
    {
        template<typename E, typename ReturnType = E>
        using IfEnabled = std::enable_if_t<EnumBitmask<E>::value, ReturnType>;
    }

    template<typename E>
    constexpr detail::IfEnabled<E> operator| (E lhs, E rhs)
    {
        using T = std::underlying_type_t<E>;

        return static_cast<E>(
            static_cast<T>(lhs) | static_cast<T>(rhs)
        );
    }

    template<typename E>
    constexpr detail::IfEnabled<E> operator& (E lhs, E rhs)
    {
        using T = std::underlying_type_t<E>;

        return static_cast<E>(
            static_cast<T>(lhs) & static_cast<T>(rhs)
        );
    }

    template<typename E>
    constexpr detail::IfEnabled<E> operator^ (E lhs, E rhs)
    {
        using T = std::underlying_type_t<E>;

        return static_cast<E>(
            static_cast<T>(lhs) & static_cast<T>(rhs)
        );
    }

    template<typename E>
    constexpr detail::IfEnabled<E> operator~ (E value)
    {
        using T = std::underlying_type_t<E>;

        return static_cast<E>(
            ~static_cast<T>(value)
        );
    }

    namespace bits
    {
        template<typename E>
        constexpr detail::IfEnabled<E> set(E value, E flag)
        {
            return value | flag;
        }

        template<typename E, size_t N>
        constexpr detail::IfEnabled<E> set(E value, std::array<E, N> flags)
        {
            for (auto&& flag : flags)
                value = value | flag;

            return value;
        }

        template<typename E>
        constexpr detail::IfEnabled<E> clear(E value, E flag)
        {
            return value & ~flag;
        }

        template<typename E, size_t N>
        constexpr detail::IfEnabled<E> clear(E value, std::array<E, N> flags)
        {
            for (auto&& flag : flags)
                value = value & ~flag;

            return value;
        }

        template<typename E>
        constexpr detail::IfEnabled<E> toggle(E value, E flag)
        {
            return value ^ flag;
        }

        template<typename E, size_t N>
        constexpr detail::IfEnabled<E> toggle(E value, std::array<E, N> flags)
        {
            for (auto&& flag : flags)
                value = value ^ flag;

            return value;
        }

        template<typename E>
        constexpr detail::IfEnabled<E, bool> isSet(E value, E flag)
        {
            return (value & flag) == flag;
        }
    }
}
