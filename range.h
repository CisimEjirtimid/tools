#pragma once

namespace cisim
{
    template <typename T>
    struct range
    {
        T from = T{ 0 };
        T to = T{ 0 };

        T span() const
        {
            return to - from;
        }

        bool operator==(const range<T>& other) const
        {
            return std::tie(from, to) == std::tie(other.from, other.to);
        }
    };

    using irange = range<int>;
    using frange = range<float>;
    using drange = range<double>;
}