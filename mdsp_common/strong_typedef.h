#pragma once

// Example:
// using Timestamp = StrongTypedef<uint64_t, struct _TimestampTag>;
// using ByteOffset = StrongTypedef<uint64_t, struct _ByteOffsetTag>;
//
// In this case Timestamp and ByteOffset will be completely different types even though both are typedefs of uint64_t.
// This is done by tagging them with a phantom type that's only defined as the second template parameter.
template<typename ValueType, typename Tag>
class StrongTypedef
{
public:
    using type = ValueType;

private:
    ValueType _value;

public:
    constexpr explicit StrongTypedef() = default;

    constexpr explicit StrongTypedef(const ValueType& value)
        : _value(value)
    {
    }

    constexpr explicit StrongTypedef(ValueType&& value)
        : _value(std::move(value))
    {
    }

    constexpr StrongTypedef(const StrongTypedef& other) = default;
    constexpr StrongTypedef(StrongTypedef&& other) noexcept = default;

    StrongTypedef& operator=(const StrongTypedef& other) = default;
    StrongTypedef& operator=(StrongTypedef&& other) noexcept = default;

    constexpr operator ValueType() const
    {
        return _value;
    }

    ValueType* operator->()
    {
        return &_value;
    }

    const ValueType* operator->() const
    {
        return &_value;
    }
};
