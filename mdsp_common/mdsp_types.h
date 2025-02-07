/********************************************************************
    created:    2007/12/06
    created:    6:12:2007   10:20
    filename:     types.h
    file base:    types
    file ext:    h
    author:        Ivan.Velickovic

    purpose:    This file sholuld establish universal system of data
                types (both native and user specific) for mdsp designs.
*********************************************************************/
#pragma once
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <chrono>
#include <vector>
#include <string.h>
#include <algorithm>
#include <ostream>
#include <climits>
#include <cstdint>
#include <cstring>
#include <sstream>
#include "timestamp.h"
#include "static_vec.h"
#include "static_mx.h"

#undef min
#undef max

#ifndef types_h__
#define types_h__

namespace mdsp
{
    typedef unsigned int       uint;
    typedef unsigned short     ushort;
    typedef unsigned char      uchar;
    typedef int32_t            int32;
    typedef uint32_t           uint32;
    typedef int64_t            int64;
    typedef uint64_t           uint64;

    typedef void* MdspWindowHandle;
    typedef void* D3DTextureHandle;

    enum class DataType { Float, Byte };

    namespace math
    {
        constexpr double PI = 3.14159265358979323846;
        constexpr double OneEightyOverPi = 180.0 / PI;
        constexpr double PiOverOneEighty = PI / 180.0;

        template<typename T>
        constexpr inline T rad2deg(T angleInRad)
        {
            return angleInRad * static_cast<T>(OneEightyOverPi);
        }

        template<typename T>
        constexpr inline T deg2rad(T angleInDeg)
        {
            return angleInDeg * static_cast<T>(PiOverOneEighty);
        }

        template<typename T>
        inline T clamp(T val, T min, T max)
        {
            const T temp = val < min ? min : val;
            return temp > max ? max : temp;
        }

        template<typename T>
        inline T clampUp(T val, T min)
        {
            return val < min ? min : val;
        }

        template<typename T>
        inline T clampDown(T val, T max)
        {
            return val > max ? max : val;
        }

        // implementation exploiting union definition of float
        inline float pow2f(int x)
        {
            union {
                float retval;
                struct {
                    uint32_t mantissa : 23;
                    uint32_t exponent : 8;
                    uint32_t sign : 1;
                } parts;
            } u;
            u.retval = 1.f;
            u.parts.exponent += x;
            return u.retval;
        }

        // implementation exploiting union definition of double
        inline double pow2d(int x)
        {
            union {
                double retval;
                struct {
                    uint64_t mantissa : 52;
                    uint64_t exponent : 11;
                    uint64_t sign : 1;
                } parts;
            } u;
            u.retval = 1.0;
            u.parts.exponent += x;
            return u.retval;
        }
    }

    template<typename T>
    inline void mdsp_swap(T& a, T& b)
    {
        T tmp = a;
        a = b;
        b = tmp;
    }

    template<size_t N>
    double decimals(double value)
    {
        int i;

        auto q = std::pow(10, N);

        if (value >= 0)
            i = static_cast<int>(value * q + 0.5);
        else
            i = static_cast<int>(value * q - 0.5);

        return i / q;
    }

    template<typename T>
    struct SizeT
    {
        T height;
        T width;

        // necessary because of the CLI projects supporting only up to C++17 standard
        #ifdef __cpp_impl_three_way_comparison
        constexpr auto operator<=>(const SizeT&) const = default;
        #endif

        constexpr SizeT()
            : height(static_cast<T>(0))
            , width(static_cast<T>(0))
        {}

        constexpr SizeT(T aHeight, T aWidth)
            : height(aHeight)
            , width(aWidth)
        {
            assert(height >= 0);
            assert(width >= 0);
        }

        template<typename U, typename V
            , typename Enable = std::enable_if_t<std::is_arithmetic_v<U> && std::is_arithmetic_v<V>>>
        constexpr SizeT(U aHeight, V aWidth)
            : SizeT<T>(static_cast<T>(aHeight), static_cast<T>(aWidth))
        {
        }

        constexpr SizeT(const SizeT& other)
            : SizeT<T>(other.height, other.width)
        {}

        constexpr SizeT(SizeT&& other) noexcept
            : height(std::move(other.height))
            , width(std::move(other.width))
        {}

        constexpr SizeT<T>& operator=(const SizeT<T>& other)
        {
            height = other.height;
            width = other.width;

            return *this;
        }

        constexpr SizeT<T>& operator=(SizeT<T>&& other) noexcept
        {
            height = std::move(other.height);
            width = std::move(other.width);

            return *this;
        }

        bool operator==(const SizeT<T>& size) const
        {
            return height == size.height && width == size.width;
        }

        bool operator!=(const SizeT<T>& size) const
        {
            return !(*this == size);
        }

        bool operator>(const SizeT<T>& size) const
        {
            return height > size.height && width > size.width;
        }

        T area() const
        {
            return height * width;
        }

        constexpr operator bool() const
        {
            return width > 0 && height > 0;
        }

        SizeT flip() const
        {
            return SizeT(width, height);
        }

        SizeT subsample2x() const
        {
            assert(height > 0);
            assert(width > 0);

            auto h = height / 2;
            auto w = width / 2;

            h = (h > 1) ? h : 1;
            w = (w > 1) ? w : 1;

            return SizeT(h, w);
        }

        vec2<T> asvec2() const
        {
            return vec2{ height, width };
        }

        bool normalized() const
        {
            return !(int(width) & 0x01) && !(int(height) & 0x01);
        }

        SizeT normalize() const
        {
            return SizeT(T(int(height) & ~0x01), T(int(width) & ~0x01));
        }

        template<typename R>
        SizeT<R> to() const
        {
            return SizeT<R>{ R(height), R(width) };
        }

        template<typename R>
        operator SizeT<R>() const
        {
            return this->to<R>();
        }
    };

    using Size = SizeT<int>;
    using SizeU = SizeT<uint>;
    using SizeS = SizeT<size_t>;
    using SizeF = SizeT<float>;
    using SizeD = SizeT<double>;

    template<typename T>
    class AngleT
    {
    public:
        enum class Type
        {
            Absolute,
            Relative
        };

        Type type;

    private:
        T _angleInDegrees;

        Type resultingType(const AngleT<T>& other) const
        {
            return type == Type::Relative && other.type == Type::Relative ? Type::Relative : Type::Absolute;
        }

        T mod(T degrees)
        {
            if constexpr (std::is_floating_point_v<T>)
                return std::fmod(degrees, T(360.0));
            else
                return degrees % 360;
        }

    public:
        AngleT() = default;

        AngleT(T degrees, Type t = Type::Relative)
            : _angleInDegrees(mod(degrees))
            , type(t)
        {
        }

        AngleT(const AngleT& other) = default;

        AngleT(const AngleT& other, Type newType)
            : _angleInDegrees(other._angleInDegrees)
            , type(newType)
        {
        }

        T degrees() const
        {
            return _angleInDegrees;
        }

        T radians() const
        {
            return math::deg2rad(_angleInDegrees);
        }

        bool operator==(const AngleT<T>& other) const
        {
            return std::tie(this->_angleInDegrees, this->type)
                == std::tie(other._angleInDegrees, other.type);
        }

        bool operator!=(const AngleT<T>& other) const
        {
            return !(*this == other);
        }

        bool operator>(const AngleT<T>& other) const
        {
            return this->_angleInDegrees > other._angleInDegrees;
        }

        bool operator>=(const AngleT<T>& other) const
        {
            return this->_angleInDegrees >= other._angleInDegrees;
        }

        bool operator<(const AngleT<T>& other) const
        {
            return this->_angleInDegrees < other._angleInDegrees;
        }

        bool operator<=(const AngleT<T>& other) const
        {
            return this->_angleInDegrees <= other._angleInDegrees;
        }

        AngleT<T> operator-() const
        {
            return AngleT<T>(-this->_angleInDegrees, this->type);
        }

        AngleT<T> operator+(const AngleT<T>& other) const
        {
            return AngleT<T>{ this->_angleInDegrees + other._angleInDegrees, resultingType(other) };
        }

        AngleT<T>& operator+=(const AngleT<T>& other)
        {
            this->_angleInDegrees = mod(this->_angleInDegrees + other._angleInDegrees);
            return *this;
        }

        AngleT<T> operator-(const AngleT<T>& other) const
        {
            return AngleT<T>{ this->_angleInDegrees - other._angleInDegrees, resultingType(other) };
        }

        AngleT<T>& operator-=(const AngleT<T>& other)
        {
            this->_angleInDegrees = mod(this->_angleInDegrees - other._angleInDegrees);
            return *this;
        }

        AngleT<T> operator*(T factor) const
        {
            return AngleT<T>{ this->_angleInDegrees * factor, this->type };
        }

        AngleT<T>& operator*=(T factor)
        {
            this->_angleInDegrees = mod(this->_angleInDegrees * factor);
            return *this;
        }

        AngleT<T> operator/(T factor) const
        {
            return AngleT<T>{ this->_angleInDegrees / factor, this->type };
        }

        AngleT<T>& operator/=(T factor)
        {
            this->_angleInDegrees = mod(this->_angleInDegrees / factor);
            return *this;
        }

        bool isBetween(const AngleT<T>& lowerBound, const AngleT<T>& upperBound)
        {
            return *this >= lowerBound && *this < upperBound;
        };

        static AngleT<T> betweenVectors(const vec2<T>& u, const vec2<T>& v)
        {
            return AngleT{ math::rad2deg(u.angle(v)) };
        }

        static AngleT<T> betweenVectors(const Vec3<T>& u, const Vec3<T>& v)
        {
            return AngleT{ math::rad2deg(u.angle(v)) };
        }

        static AngleT<T> betweenVectorsInPlane(const Vec3<T>& u, const Vec3<T>& v, const Vec3<T>& normal)
        {
            return AngleT{ math::rad2deg(u.angleInPlane(v, normal)) };
        }

        static inline AngleT<T> zero() { return AngleT{ T(0.0) }; }

        static inline AngleT<T> pi() { return AngleT{ T(180.0) }; }
        static inline AngleT<T> piHalf() { return AngleT{ T(90.0) }; }
        static inline AngleT<T> piThird() { return AngleT{ T(60.0) }; }
        static inline AngleT<T> piQuarter() { return AngleT{ T(45.0) }; }
        static inline AngleT<T> piSixth() { return AngleT{ T(30.0) }; }
        static inline AngleT<T> piTwelfth() { return AngleT{ T(15.0) }; }

        static inline const auto oneEighty = pi;
        static inline const auto ninety = piHalf;
        static inline const auto sixty = piThird;
        static inline const auto fortyFive = piQuarter;
        static inline const auto thirty = piSixth;
        static inline const auto fifteen = piTwelfth;
    };

    using Angle = AngleT<double>;
    using AngleF = AngleT<float>;

    enum class Bounds
    {
        INCLUSIVE,
        EXCLUSIVE
    };

    // static_assert has to be used because of C++/CLI
    template<typename T, Bounds type>
    struct RectT
    {
        static_assert(std::is_arithmetic_v<T>, "Specified template argument is not an arithmetic type");

        T top;
        T left;
        T bottom;
        T right;

        constexpr RectT() // default-constructed Rect is invalid
            : top(T(1))
            , left(T(1))
            , bottom(T(0))
            , right(T(0))
        {}

        template<typename R = T>
        constexpr RectT(R Top, R Left, R Bottom, R Right)
            : top(T(Top))
            , left(T(Left))
            , bottom(T(Bottom))
            , right(T(Right))
        {}

        template<typename R = T>
        constexpr explicit RectT(const RectT<R, type>& other)
            : top(T(other.top))
            , left(T(other.left))
            , bottom(T(other.bottom))
            , right(T(other.right))
        {
        }

        template<typename R = T>
        constexpr explicit RectT(RectT<R, type>&& other) noexcept
            : top(T(std::move(other.top)))
            , left(T(std::move(other.left)))
            , bottom(T(std::move(other.bottom)))
            , right(T(std::move(other.right)))
        {
        }

        template<typename R = T>
        constexpr RectT<T, type>& operator=(const RectT<R, type>& other)
        {
            if (*this != other)
            {
                top = T(other.top);
                left = T(other.left);
                bottom = T(other.bottom);
                right = T(other.right);
            }

            return *this;
        }

        template<typename R = T>
        constexpr RectT<T, type>& operator=(RectT<R, type>&& other) noexcept
        {
            if (*this != other)
            {
                top = T(std::move(other.top));
                left = T(std::move(other.left));
                bottom = T(std::move(other.bottom));
                right = T(std::move(other.right));
            }

            return *this;
        }

        template<typename R = T>
        constexpr explicit RectT(const SizeT<R>& size)
            : top(T(0))
            , left(T(0))
            , bottom(T(size.height - 1))
            , right(T(size.width - 1))
        {
            if constexpr (std::is_integral_v<R> && type == Bounds::EXCLUSIVE)
            {
                ++bottom;
                ++right;
            }
        }

        template<typename R = T, Bounds btype = type, typename Enable = std::enable_if_t<btype == Bounds::INCLUSIVE>>
        RectT<R, Bounds::EXCLUSIVE> toExclusive() const
        {
            RectT<R, Bounds::EXCLUSIVE> res(top, left, bottom, right);

            if constexpr (std::is_integral_v<R>)
            {
                ++res.bottom;
                ++res.right;
            }

            return res;
        }

        template<typename R = T, Bounds btype = type, typename Enable = std::enable_if_t<btype == Bounds::EXCLUSIVE>>
        RectT<R, Bounds::INCLUSIVE> toInclusive() const
        {
            RectT<R, Bounds::INCLUSIVE> res(top, left, bottom, right);

            if constexpr (std::is_integral_v<R>)
            {
                --res.bottom;
                --res.right;
            }

            return res;
        }

        template<typename R = T>
        SizeT<R> size() const
        {
            if constexpr (type == Bounds::INCLUSIVE)
                return SizeT<R>(R(bottom - top + 1), R(right - left + 1));
            else
                return SizeT<R>(R(bottom - top), R(right - left));
        }

        bool isNull() const noexcept
        {
            bool boundsNull = top == T(0) && bottom == T(0) && left == T(0) && right == T(0);

            if constexpr (type == Bounds::INCLUSIVE)
                return boundsNull || top > bottom || left > right;
            else
                return boundsNull || top >= bottom || left >= right;
        }

        operator bool() const noexcept
        {
            return !isNull();
        }

        T height() const
        {
            if constexpr (type == Bounds::INCLUSIVE)
                return std::abs(bottom - top + T(1));
            else
                return std::abs(bottom - top);
        }

        T width() const
        {
            if constexpr (type == Bounds::INCLUSIVE)
                return std::abs(right - left + T(1));
            else
                return std::abs(right - left);
        }

        template<typename R = double>
        R diagonal() const
        {
            R width  = R(this->width());
            R height = R(this->height());

            return R(sqrt(width * width + height * height));
        }

        template<typename R = size_t>
        R area() const
        {
            return R(width()) * R(height());
        }

        template<typename R = T>
        vec2<R> topLeft() const
        {
            return vec2<R>(R(left), R(top));
        }

        template<typename R = T>
        vec2<R> topRight() const
        {
            return vec2<R>(R(right), R(top));
        }

        template<typename R = T>
        vec2<R> bottomLeft() const
        {
            return vec2<R>(R(left), R(bottom));
        }

        template<typename R = T>
        vec2<R> bottomRight() const
        {
            return vec2<R>(R(right), R(bottom));
        }

        template<typename R = T>
        vec2<R> centroid() const
        {
            if constexpr (type == Bounds::INCLUSIVE)
                return vec2<R>(R((left + right) / 2.0), R((top + bottom) / 2.0));
            else
                return vec2<R>(R((left + right - 1) / 2.0), R((top + bottom - 1) / 2.0));
        }

        bool operator==(const RectT& r) const
        {
            return    top == r.top    &&
                   bottom == r.bottom &&
                     left == r.left   &&
                    right == r.right;
        }

        bool operator!=(const RectT& r) const
        {
            return !(*this == r);
        }

        RectT operator+(const RectT& r) const
        {
            RectT res;

            res.top    = (top    < r.top)    ? top    : r.top;
            res.left   = (left   < r.left)   ? left   : r.left;
            res.bottom = (bottom > r.bottom) ? bottom : r.bottom;
            res.right  = (right  > r.right)  ? right  : r.right;

            return res;
        }

        RectT operator+(const vec2<T>& pt) const
        {
            return RectT(top + pt.y, left + pt.x, bottom + pt.y, right + pt.x);
        }

        RectT operator-(const vec2<T>& pt) const
        {
            return RectT(top - pt.y, left - pt.x, bottom - pt.y, right - pt.x);
        }

        RectT operator*(const RectT& r) const
        {
            RectT res;

            res.top    = (top    > r.top)    ? top    : r.top;
            res.left   = (left   > r.left)   ? left   : r.left;
            res.bottom = (bottom < r.bottom) ? bottom : r.bottom;
            res.right  = (right  < r.right)  ? right  : r.right;

            return res;
        }

        RectT intersect(const RectT& r) const
        {
            RectT areaBetween = (*this)*r;

            if (areaBetween.isNull())
                return RectT();
            else
                return areaBetween;
        }

        bool intersects(const RectT& r, float intersectionAreaThreshold) const
        {
            RectT intersection = this->intersect(r);
            if (intersection.isNull())
                return false;

            float intersectArea = static_cast<float>(intersection.area());
            float unionArea = static_cast<float>(1 + r.area() + this->area() - intersectArea);
            bool areIntersecting = (intersectArea / unionArea) > intersectionAreaThreshold;

            return areIntersecting;
        }

        double intersectionOverUnion(const RectT& r, double intersectionAreaThreshold = 0.0) const
        {
            RectT intersection = this->intersect(r);

            if (intersection.isNull())
                return 0.0;

            double intersectArea = static_cast<double>(intersection.area());
            double unionArea = static_cast<double>(1 + r.area() + this->area() - intersectArea);

            auto iou = ((intersectArea / unionArea) >= intersectionAreaThreshold) ? (intersectArea / unionArea) : 0.0;

            return iou;
        }

        RectT operator-(const RectT& r) const
        {
            RectT res;

            res.top    = top - r.top;
            res.left   = left - r.left;
            res.bottom = bottom - r.top;
            res.right  = right - r.left;

            return res;
        }

        template<typename R = T>
        bool ptInRect(const vec2<R>& p) const
        {
            if constexpr (type == Bounds::INCLUSIVE)
                return T(p.x) >= left && T(p.x) <= right && T(p.y) >= top && T(p.y) <= bottom;
            else
                return T(p.x) >= left && T(p.x) < right  && T(p.y) >= top && T(p.y) < bottom;
        }

        template<typename R = T>
        bool ptInRect(R x, R y) const
        {
            if constexpr (type == Bounds::INCLUSIVE)
                return T(x) >= left && T(x) <= right && T(y) >= top && T(y) <= bottom;
            else
                return T(x) >= left && T(x) < right  && T(y) >= top && T(y) < bottom;
        }

        template<typename R = T>
        bool rectInRect(const RectT<R, type>& r) const
        {
            return ptInRect(r.template topLeft<R>()) &&
                   ptInRect(r.template bottomRight<R>());
        }

        void normalize()
        {
            if (top > bottom) mdsp_swap(top, bottom);
            if (left > right) mdsp_swap(left, right);
        }

        RectT subsample2x() const
        {
            if constexpr (type == Bounds::INCLUSIVE)
                return RectT(top / 2,
                             left / 2,
                             std::max(top / 2, top / 2 + height() / 2 - 1),
                             std::max(left / 2, left / 2 + width() / 2 - 1));
            else
                return RectT(top / 2, left / 2, bottom / 2, right / 2);
        }

        RectT upsample2x() const
        {
            if constexpr (type == Bounds::INCLUSIVE)
                return RectT(top * 2,
                             left * 2,
                             std::max(top * 2, top * 2 + height() * 2 - 1),
                             std::max(left * 2, left * 2 + width() * 2 - 1));
            else
                return RectT(top * 2, left * 2, bottom * 2, right * 2);
        }

        RectT centralsubsample2x() const
        {
            return RectT(height() / 4,
                         width() / 4,
                         height() * 3 / 4,
                         width() * 3 / 4);
        }

        template<typename R = T>
        std::vector<vec2<R>> corners() const
        {
            std::vector<vec2<R>> res;

            //Order of points - top left, top right, bottom right, bottom left.
            res.push_back(topLeft<R>());
            res.push_back(topRight<R>());
            res.push_back(bottomRight<R>());
            res.push_back(bottomLeft<R>());

            return res;
       }

        template<typename R = T>
        std::vector<vec2<R>> centroidAndCorners() const
        {
            std::vector<vec2<R>> res;

            //Order of points - frame center, top left, top right, bottom right, bottom left.
            res.push_back(centroid<R>());
            res.push_back(topLeft<R>());
            res.push_back(topRight<R>());
            res.push_back(bottomRight<R>());
            res.push_back(bottomLeft<R>());

            return res;
        }

        RectT translate(T tx, T ty) const
        {
            return RectT(top + ty, left + tx, bottom + ty, right + tx);
        }

        RectT translate(const vec2<T>& t) const
        {
            return translate(t.x, t.y);
        }

        template<typename R = T>
        RectT translateTo(R x, R y) const
        {
            R newCentroidX = x + R(0.5);
            R newCentroidY = y + R(0.5);

            vec2<R> oldCentroid = centroid<R>();

            T tx = T(newCentroidX - oldCentroid.x);
            T ty = T(newCentroidY - oldCentroid.y);

            return RectT(top + ty, left + tx, bottom + ty, right + tx);
        }

        template<typename R = T>
        RectT scale(R sx, R sy) const
        {
            R incrementW = R(width()) * (R(1.0) - sx) / R(2.0);
            R incrementH = R(height()) * (R(1.0) - sy) / R(2.0);

            T newTop = T(top + incrementH + R(0.5));
            T newLeft = T(left + incrementW + R(0.5));
            T newBottom = T(bottom - incrementH + R(0.5));
            T newRight = T(right - incrementW + R(0.5));

            return RectT(newTop, newLeft, newBottom, newRight);
        }

        template<typename R = T>
        RectT scale(const vec2<T>& s) const
        {
            return scale(s.x, s.y);
        }

        template<typename R = T>
        RectT scale(R s) const
        {
            return scale(s, s);
        }

        // Shrink rect by subtracting 'amount' points from each side
        RectT shrink(T amount)
        {
            return RectT(top + amount, left + amount, bottom - amount, right - amount);
        }

        inline friend std::ostream& operator<<(std::ostream& output, const RectT<T, type>& rect)
        {
            return output << "[(" << rect.top << ", " << rect.left << ") (" << rect.bottom << ", " << rect.right << ")]";
        }

        Vec4<T> lbrt() const
        {
            return Vec4<T>{ left, top, right, bottom };
        }

        template<typename R = T>
        vec2<R> clip(const vec2<R>& point) const
        {
            return vec2<R>{ std::clamp(point.x, R(left), R(right)), std::clamp(point.y, R(top), R(bottom)) };
        }
    };

    using Rect = RectT<int, Bounds::INCLUSIVE>;

    template<typename T>
    using Rectangle = RectT<T, Bounds::EXCLUSIVE>;

    template<typename T>
    using RectI = RectT<T, Bounds::INCLUSIVE>;

    template<typename T>
    using RectE = RectT<T, Bounds::EXCLUSIVE>;

    using RectIi = RectI<int>;
    using RectIui = RectI<unsigned int>;
    using RectEi = RectE<int>;
    using RectEui = RectE<unsigned int>;
    using RectEf = RectE<float>;
    using RectEd = RectE<double>;

    template<typename T>
    Rectangle<T> boundingRect(const std::vector<vec2<T>>& points)
    {
        Rectangle<T> res(T(points[0].y), T(points[0].x), T(points[0].y), T(points[0].x));

        for (auto p : points)
        {
            res.top = std::min(res.top, T(p.y));
            res.left = std::min(res.left, T(p.x));
            res.bottom = std::max(res.bottom, T(p.y));
            res.right = std::max(res.right, T(p.x));
        }

        return res;
    }

    struct Polygon
    {
        std::vector<int2> points;

        bool isEmpty() const
        {
            return points.empty();
        }

        Rect getBounds()
        {
            auto xMin = INT_MIN;
            auto xMax = INT_MAX;
            auto yMin = INT_MIN;
            auto yMax = INT_MAX;

            for (auto& p : points)
            {
                auto x = p.x;
                auto y = p.y;

                if (x < xMin) xMin = x;
                if (x > xMax) xMax = x;

                if (y < yMin) yMin = y;
                if (y > yMax) yMax = y;
            }

            return Rect(yMin, xMin, yMax, xMax);
        }

         bool containsPoint(const int2& pt)
         {
             if (points.empty())
                 return false;

             auto isIn = false;
             auto bounds = getBounds();

             if (bounds.ptInRect(pt))
             {
                 for (size_t i = 0, j = (points.size() - 1); i < points.size(); j = i++)
                 {
                     if ((points[i].y <= pt.y && pt.y < points[j].y || points[j].y <= pt.y && pt.y < points[i].y) &&
                         pt.x < (points[j].x - points[i].x) * (pt.y - points[i].y) / (points[j].y - points[i].y) + points[i].x)
                     {
                         isIn = !isIn;
                     }
                 }
             }

             return isIn;
         }

         Polygon scale(float scaleX, float scaleY)
         {
             Polygon dstPolygon;

             for (auto& p : points)
                 dstPolygon.points.push_back(int2{ int(p.x * scaleX + 0.5f), int(p.y * scaleY + 0.5f) });

             return dstPolygon;
         }
    };

    template<typename T>
    struct Polygon3D
    {
        std::vector<Vec3<T>> points;

        bool isEmpty() const
        {
            return points.empty();
        }
    };

    struct BGRA
    {
        uchar b = 0;
        uchar g = 0;
        uchar r = 0;
        uchar a = 0;

        BGRA() = default;
        BGRA(const BGRA&) = default;
        BGRA(BGRA&&) noexcept = default;
        BGRA& operator=(const BGRA&) = default;
        BGRA& operator=(BGRA&&) noexcept = default;

        BGRA(uchar b, uchar g, uchar r, uchar a)
            : b(b), g(g), r(r), a(a)
        {}

        BGRA(std::string color)
        {
            //color should be in format of #AARRGGBB

            if (color.at(0) == '#')
                color = color.erase(0, 1);

            if (color.size() == 8)
            {
                a = uchar(std::stoi(color.substr(0, 2), 0, 16));
                r = uchar(std::stoi(color.substr(2, 2), 0, 16));
                g = uchar(std::stoi(color.substr(4, 2), 0, 16));
                b = uchar(std::stoi(color.substr(6, 2), 0, 16));
            }
        }

        // Operators necessary for linear interpolation
        BGRA operator*(float rhs)
        {
            return {
                uchar(math::clamp(rhs * b, 0.0f, 255.0f)),
                uchar(math::clamp(rhs * g, 0.0f, 255.0f)),
                uchar(math::clamp(rhs * r, 0.0f, 255.0f)),
                uchar(math::clamp(rhs * a, 0.0f, 255.0f))
            };
        }

        BGRA operator+(const BGRA& other_bgra)
        {
            return {
                uchar(math::clamp((int)b + other_bgra.b, 0, 255)),
                uchar(math::clamp((int)g + other_bgra.g, 0, 255)),
                uchar(math::clamp((int)r + other_bgra.r, 0, 255)),
                uchar(math::clamp((int)a + other_bgra.a, 0, 255))
            };
        }

        template<typename T>
        std::array<T, 4> to() const
        {
            return { T(b), T(g), T(r), T(a) };
        }

        template<typename T = uchar>
        Vec4<T> toVec() const
        {
            return uchar4{ r, g, b, a }.to<T>();
        }

        std::string toString() const
        {
            std::stringstream ss;
            ss << '#' << std::hex << std::setfill('0') <<
                std::setw(2) << int(a) <<
                std::setw(2) << int(r) <<
                std::setw(2) << int(g) <<
                std::setw(2) << int(b);

            return ss.str();
        }
    };

    inline bool operator==(const BGRA& lhs, const BGRA& rhs)
    {
        return std::tie(lhs.b, lhs.g, lhs.r, lhs.a)
            == std::tie(rhs.b, rhs.g, rhs.r, rhs.a);
    }

    inline BGRA operator*(float lhs, const BGRA& rhs)
    {
        return {
            uchar(math::clamp(lhs * rhs.b, 0.0f, 255.0f)),
            uchar(math::clamp(lhs * rhs.g, 0.0f, 255.0f)),
            uchar(math::clamp(lhs * rhs.r, 0.0f, 255.0f)),
            uchar(math::clamp(lhs * rhs.a, 0.0f, 255.0f))
        };
    }
}

#endif // types_h__
