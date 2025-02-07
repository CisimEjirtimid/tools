#pragma once
#include <cstring>

namespace mdsp
{
    inline float fNaN()
    {
        const int32_t binaryNaN = 0x7fc0'0000;
        float tmp;
        std::memcpy(&tmp, &binaryNaN, sizeof(float));
        return tmp;
    }

    inline double dNaN()
    {
        const int64_t binaryNaN = 0x7ff8'0000'0000'0000;
        double tmp;
        std::memcpy(&tmp, &binaryNaN, sizeof(double));
        return tmp;
    }

    inline long double ldNaN()
    {
        return std::numeric_limits<long double>::quiet_NaN();
    }

    template<typename T>
    inline bool isNaN(T x)
    {
        return isNaN(static_cast<double>(x)); // not sure if relevant, but integer types get cast to double to prevent 
    }                                         // loss of information (isNaN() is called for integers inside dbg_utils.cpp)

    template<>
    inline bool isNaN(float f)
    {
        int32_t intF;
        std::memcpy(&intF, &f, sizeof(float));
        return (intF & 0x7fc0'0000) == 0x7fc0'0000;
    }

    template<>
    inline bool isNaN(double d)
    {
        int64_t intD;
        std::memcpy(&intD, &d, sizeof(double));
        return (intD & 0x7ff8'0000'0000'0000) == 0x7ff8'0000'0000'0000;
    }

    template<>
    inline bool isNaN(long double ld)
    {
        return std::isnan(ld);
    }
}
