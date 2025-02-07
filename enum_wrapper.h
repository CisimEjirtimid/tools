#pragma once
#include <type_traits>

namespace cisim
{
    template <typename E>
    concept IsEnum = std::is_enum_v<E>;

    template <typename E, typename UnderlyingT = uint8_t>
        requires IsEnum<E>
    class EnumWrapper
    {
    public:
        using Enum = E;

        EnumWrapper() = default;
        constexpr EnumWrapper(Enum value)
            : _value(value)
        {}
        constexpr EnumWrapper(UnderlyingT value)
            : _value(Enum(value))
        {}

        constexpr EnumWrapper& operator=(Enum value)
        {
            _value = value;
            return *this;
        }
        constexpr EnumWrapper& operator=(UnderlyingT value)
        {
            _value = Enum(value);
            return *this;
        }

        // Allow switch and comparisons.
        constexpr operator Enum() const { return _value; }
        constexpr UnderlyingT underlying() const { return UnderlyingT(_value); }

        // Prevent usage: if(Enum)
        explicit operator bool() const = delete;

    private:
        Enum _value;
    };
}
