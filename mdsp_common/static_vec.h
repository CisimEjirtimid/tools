#pragma once
#include <assert.h>
#include <cmath>
#include <algorithm>
#include <type_traits>
#include "mdsp_nan.h"

namespace mdsp
{
    // static_assert has to be used because of C++/CLI
    template<typename T>
    struct vec2
    {
        static_assert(std::is_arithmetic_v<T>, "Specified template argument is not an arithmetic type");

        using value_type = T;

        T x, y;

        explicit constexpr vec2(T a = T{ 0 }, T b = T{ 0 })
            : x{ a }, y{ b }
        {}

        explicit constexpr vec2(const T arr[2])
            : x{ arr[0] }, y{ arr[1] }
        {}

        constexpr vec2(const vec2<T>& v)
            : x{ v.x }, y{ v.y }
        {}

        constexpr vec2(vec2<T>&& v) noexcept
            : x(std::move(v.x))
            , y(std::move(v.y))
        {}

        vec2<T>& operator=(const T arr[2])
        {
            x = arr[0];
            y = arr[1];

            return *this;
        }

        vec2<T>& operator=(const vec2<T>& v)
        {
            if (this != &v)
            {
                x = v.x;
                y = v.y;
            }

            return *this;
        }

        vec2<T>& operator=(vec2<T>&& v) noexcept
        {
            x = std::move(v.x);
            y = std::move(v.y);

            return *this;
        }

        bool isnan() const
        {
            return isNaN(x) || isNaN(y);
        }

        bool operator==(const vec2<T>& v)      const   { return x == v.x && y == v.y; }
        bool operator!=(const vec2<T>& v)      const   { return !(*this == v); }

        T& operator[](int i) { assert(i >= 0 && i < 2); return i == 0 ? x : y; }
        const   T& operator[](int i) const { assert(i >= 0 && i < 2); return i == 0 ? x : y; }

        template<typename R>
        vec2<R> to() const { return vec2<R>{ static_cast<R>(x), static_cast<R>(y) }; }

        vec2<T> operator-()                     const   { return vec2<T>{ -x, -y }; }
        vec2<T> operator+(const vec2<T>& v)     const   { return vec2<T>{ x + v.x, y + v.y }; }
        vec2<T> operator-(const vec2<T>& v)     const   { return vec2<T>{ x - v.x, y - v.y }; }
        vec2<T> operator*(const vec2<T>& v)     const   { return vec2<T>{ x * v.x, y * v.y }; }
        vec2<T> operator/(const vec2<T>& v)     const   { return vec2<T>{ x / v.x, y / v.y }; }
        vec2<T> operator+(T c)                  const   { return vec2<T>{ x + c, y + c }; }
        vec2<T> operator-(T c)                  const   { return vec2<T>{ x - c, y - c }; }
        vec2<T> operator*(T c)                  const   { return vec2<T>{ x * c, y * c }; }
        vec2<T> operator/(T c)                  const   { return vec2<T>{ x / c, y / c }; }
        vec2<T>& operator+=(const vec2<T>& v)           { return *this = *this + v; }
        vec2<T>& operator-=(const vec2<T>& v)           { return *this = *this - v; }
        vec2<T>& operator*=(const vec2<T>& v)           { return *this = *this * v; }
        vec2<T>& operator/=(const vec2<T>& v)           { return *this = *this / v; }
        vec2<T>& operator+=(T c)                        { return *this = *this + c; }
        vec2<T>& operator-=(T c)                        { return *this = *this - c; }
        vec2<T>& operator*=(T c)                        { return *this = *this * c; }
        vec2<T>& operator/=(T c)                        { return *this = *this / c; }

        static T dot(const vec2<T>& u, const vec2<T>& v)
        {
            return static_cast<T>(u.x * v.x + u.y * v.y);
        }

        static vec2<T> normalize(const vec2<T>& v)
        {
            if (T length = v.l2norm(); length > T{ 0 })
                return vec2<T>{ v.x / length, v.y / length };

            return v;
        }

        static T manhattanDistance(const vec2<T>& u, const vec2<T>& v)
        {
            return static_cast<T>((u - v).l1norm());
        }

        static T euclideanDistance(const vec2<T>& u, const vec2<T>& v)
        {
            return static_cast<T>((u - v).l2norm());
        }

        static T angle(const vec2<T>& u, const vec2<T>& v)
        {
            auto uN = u.normalize();
            auto vN = v.normalize();

            auto dot = uN.dot(vN);
            auto det = uN.x * vN.y - uN.y * vN.x; // determinant

            return std::atan2(det, dot);
        }

        T l1norm() const
        {
            return static_cast<T>(std::abs(x) + std::abs(y));
        }

        T l2norm() const
        {
            return static_cast<T>(std::sqrt(dot(*this)));
        }

        T dot(const vec2<T>& v) const { return dot(*this, v); }
        vec2<T> normalize() const { return normalize(*this); }
        T manhattanDistance(const vec2<T>& v) const { return manhattanDistance(*this, v); }
        T euclideanDistance(const vec2<T>& v) const { return euclideanDistance(*this, v); }
        T angle(const vec2<T>& v) const { return angle(*this, v); }

        vec2<T> floor() const
        {
            return vec2<T>{ std::floor(x), std::floor(y) };
        }

        vec2<T> ceil() const
        {
            return vec2<T>{ std::ceil(x), std::ceil(y) };
        }

        vec2<T> abs() const
        {
            return vec2<T>{ std::abs(x), std::abs(y) };
        }
    };

    template<typename T> vec2<T> operator+(T c, const vec2<T>& v) { return v + c; }
    template<typename T> vec2<T> operator-(T c, const vec2<T>& v) { return vec2<T>{ c - v.x, c - v.y }; }
    template<typename T> vec2<T> operator*(T c, const vec2<T>& v) { return v * c; }
    template<typename T> vec2<T> operator/(T c, const vec2<T>& v) { return vec2<T>{ c / v.x, c / v.y }; }

    using double2 = vec2<double>;
    using float2 = vec2<float>;
    using int2 = vec2<int>;
    using uint2 = vec2<unsigned int>;
    using char2 = vec2<char>;
    using uchar2 = vec2<unsigned char>;
    using size_t2 = vec2<size_t>;

    // static_assert has to be used because of C++/CLI
    template<typename T>
    struct Vec3
    {
        static_assert(std::is_arithmetic_v<T>, "Specified template argument is not an arithmetic type");

        using value_type = T;

        T x, y, z;

        explicit constexpr Vec3(T a = T{ 0 }, T b = T{ 0 }, T c = T{ 0 })
            : x{ a }, y{ b }, z{ c }
        {}

        explicit constexpr Vec3(const T arr[3])
            : x{ arr[0] }, y{ arr[1] }, z{ arr[2] }
        {}

        explicit constexpr Vec3(const vec2<T>& v, T c = T{ 0 })
            : x{ v.x }, y{ v.y }, z{ c }
        {}

        constexpr Vec3(const Vec3<T>& v)
            : x{ v.x }, y{ v.y }, z{ v.z }
        {}

        constexpr Vec3(Vec3<T>&& v) noexcept
            : x(std::move(v.x))
            , y(std::move(v.y))
            , z(std::move(v.z))
        {}

        Vec3<T>& operator=(const T arr[3])
        {
            x = arr[0];
            y = arr[1];
            z = arr[2];

            return *this;
        }

        Vec3<T>& operator=(const Vec3<T>& v)
        {
            if (this != &v)
            {
                x = v.x;
                y = v.y;
                z = v.z;
            }

            return *this;
        }

        Vec3<T>& operator=(Vec3<T>&& v) noexcept
        {
            x = std::move(v.x);
            y = std::move(v.y);
            z = std::move(v.z);

            return *this;
        }

        bool isnan() const
        {
            return isNaN(x) || isNaN(y) || isNaN(z);
        }

        bool operator==(const Vec3<T>& v)      const   { return x == v.x && y == v.y && z == v.z; }
        bool operator!=(const Vec3<T>& v)      const   { return !(*this == v); }

        T& operator[](int i)
        {
            assert(i >= 0 && i < 3);

            switch (i)
            {
            case 0:
                return x;
            case 1:
                return y;
            default:
                return z;
            }
        }
        const T& operator[](int i) const
        {
            assert(i >= 0 && i < 3);

            switch (i)
            {
            case 0:
                return x;
            case 1:
                return y;
            default:
                return z;
            }
        }

        template<typename R>
        Vec3<R> to() const { return Vec3<R>{ static_cast<R>(x), static_cast<R>(y), static_cast<R>(z) }; }

        vec2<T> toEuclidean() const
        {
            if (z != T{ 0 })
                return vec2<T>{ x / z, y / z };

            return vec2<T>{ x, y };
        }

        vec2<T> tovec2() const
        {
            return vec2<T>{ x, y };
        }

        Vec3<T> operator-()                     const   { return Vec3<T>{ -x, -y, -z }; }
        Vec3<T> operator+(const Vec3<T>& v)     const   { return Vec3<T>{ x + v.x, y + v.y, z + v.z }; }
        Vec3<T> operator-(const Vec3<T>& v)     const   { return Vec3<T>{ x - v.x, y - v.y, z - v.z }; }
        Vec3<T> operator*(const Vec3<T>& v)     const   { return Vec3<T>{ x * v.x, y * v.y, z * v.z }; }
        Vec3<T> operator/(const Vec3<T>& v)     const   { return Vec3<T>{ x / v.x, y / v.y, z / v.z }; }
        Vec3<T> operator+(T c)                  const   { return Vec3<T>{ x + c, y + c, z + c }; }
        Vec3<T> operator-(T c)                  const   { return Vec3<T>{ x - c, y - c, z - c }; }
        Vec3<T> operator*(T c)                  const   { return Vec3<T>{ x * c, y * c, z * c }; }
        Vec3<T> operator/(T c)                  const   { return Vec3<T>{ x / c, y / c, z / c }; }
        Vec3<T>& operator+=(const Vec3<T>& v)           { return *this = *this + v; }
        Vec3<T>& operator-=(const Vec3<T>& v)           { return *this = *this - v; }
        Vec3<T>& operator*=(const Vec3<T>& v)           { return *this = *this * v; }
        Vec3<T>& operator/=(const Vec3<T>& v)           { return *this = *this / v; }
        Vec3<T>& operator+=(T c)                        { return *this = *this + c; }
        Vec3<T>& operator-=(T c)                        { return *this = *this - c; }
        Vec3<T>& operator*=(T c)                        { return *this = *this * c; }
        Vec3<T>& operator/=(T c)                        { return *this = *this / c; }

        static T dot(const Vec3<T>& u, const Vec3<T>& v)
        {
            return static_cast<T>(u.x * v.x + u.y * v.y + u.z * v.z);
        }

        static Vec3<T> normalize(const Vec3<T>& v)
        {
            if (T length = v.l2norm(); length > T{ 0 })
                return Vec3<T>{ v.x / length, v.y / length, v.z / length };
            return v;
        }

        static T manhattanDistance(const Vec3<T>& u, const Vec3<T>& v)
        {
            return static_cast<T>((u - v).l1norm());
        }

        static T euclideanDistance(const Vec3<T>& u, const Vec3<T>& v)
        {
            return static_cast<T>((u - v).l2norm());
        }

        static Vec3<T> cross(const Vec3<T>& u, const Vec3<T>& v)
        {
            return Vec3<T>{
                u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x
            };
        }

        static T angleInPlane(const Vec3<T>& u, const Vec3<T>& v, const Vec3<T>& normal)
        {
            auto uN = u.normalize();
            auto vN = v.normalize();
            auto normalN = normal.normalize();

            auto dot = uN.dot(vN);
            auto det
                = uN.x * vN.y * normalN.z
                + vN.x * normalN.y * uN.z
                + normalN.x * uN.y * vN.z
                - uN.z * vN.y * normalN.x
                - vN.z * normalN.y * uN.x
                - normalN.z * uN.y * vN.x;

            return std::atan2(det, dot);
        }

        static T angle(const Vec3<T>& u, const Vec3<T>& v)
        {
            auto uN = u.normalize();
            auto vN = v.normalize();

            return std::acos(std::clamp(uN.dot(vN), T{ -1.0 }, T{ 1.0 }));
        }

        T l1norm() const
        {
            return static_cast<T>(std::abs(x) + std::abs(y) + std::abs(z));
        }

        T l2norm() const
        {
            return static_cast<T>(std::sqrt(dot(*this)));
        }

        T dot(const Vec3<T>& v) const               { return dot(*this, v); }
        Vec3<T> normalize() const { return normalize(*this); }
        T manhattanDistance(const Vec3<T>& v) const { return manhattanDistance(*this, v); }
        T euclideanDistance(const Vec3<T>& v) const { return euclideanDistance(*this, v); }
        Vec3<T> cross(const Vec3<T>& v) const { return cross(*this, v); }
        T angleInPlane(const Vec3<T>& v, const Vec3<T>& normal) const { return angleInPlane(*this, v, normal); }
        T angle(const Vec3<T>& v) const { return angle(*this, v); }

        Vec3<T> floor() const
        {
            return Vec3<T>{ std::floor(x), std::floor(y), std::floor(z) };
        }

        Vec3<T> ceil() const
        {
            return Vec3<T>{ std::ceil(x), std::ceil(y), std::ceil(z) };
        }

        Vec3<T> abs() const
        {
            return Vec3<T>{ std::abs(x), std::abs(y), std::abs(z) };
        }

        Vec3<T> projection(const Vec3<T>& other) const
        {
            return dot(*this, other) * normalize(*this);
        }
    };

    template<typename T> Vec3<T> operator+(T c, const Vec3<T>& v) { return v + c; }
    template<typename T> Vec3<T> operator-(T c, const Vec3<T>& v) { return Vec3<T>{ c - v.x, c - v.y, c - v.z }; }
    template<typename T> Vec3<T> operator*(T c, const Vec3<T>& v) { return v * c; }
    template<typename T> Vec3<T> operator/(T c, const Vec3<T>& v) { return Vec3<T>{ c / v.x, c / v.y, c / v.z }; }

    using double3 = Vec3<double>;
    using float3 = Vec3<float>;
    using int3 = Vec3<int>;
    using uint3 = Vec3<unsigned int>;
    using char3 = Vec3<char>;
    using uchar3 = Vec3<unsigned char>;
    using size_t3 = Vec3<size_t>;

    // static_assert has to be used because of C++/CLI
    template<typename T>
    struct Vec4
    {
        static_assert(std::is_arithmetic_v<T>, "Specified template argument is not an arithmetic type");

        using value_type = T;

        T x, y, z, w;

        explicit constexpr Vec4(T a = T{ 0 }, T b = T{ 0 }, T c = T{ 0 }, T d = T{ 1 })
            : x{ a }, y{ b }, z{ c }, w{ d }
        {}

        explicit constexpr Vec4(const T arr[4])
            : x{ arr[0] }, y{ arr[1] }, z{ arr[2] }, w{ arr[3] }
        {}

        explicit constexpr Vec4(const Vec3<T>& v, T d = T{ 1 })
            : x{ v.x }, y{ v.y }, z{ v.z }, w{ d }
        {}

        explicit constexpr Vec4(const vec2<T>& u, const vec2<T>& v)
            : x{ u.x }, y{ u.y }, z{ v.x }, w{ v.y }
        {}

        explicit constexpr Vec4(const vec2<T>& v, T c = T{ 0 }, T d = T{ 1 })
            : x{ v.x }, y{ v.y }, z{ c }, w{ d }
        {}

        constexpr Vec4(const Vec4<T>& v)
            : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w }
        {}

        constexpr Vec4(Vec4<T>&& v) noexcept
            : x(std::move(v.x))
            , y(std::move(v.y))
            , z(std::move(v.z))
            , w(std::move(v.w))
        {}

        Vec4<T>& operator=(const T arr[4])
        {
            x = arr[0];
            y = arr[1];
            z = arr[2];
            w = arr[3];

            return *this;
        }

        Vec4<T>& operator=(const Vec4<T>& v)
        {
            if (this != &v)
            {
                x = v.x;
                y = v.y;
                z = v.z;
                w = v.w;
            }

            return *this;
        }

        Vec4<T>& operator=(Vec4<T>&& v) noexcept
        {
            x = std::move(v.x);
            y = std::move(v.y);
            z = std::move(v.z);
            w = std::move(v.w);

            return *this;
        }

        bool isnan() const
        {
            return isNaN(x) || isNaN(y) || isNaN(z) || isNaN(w);
        }

        bool operator==(const Vec4<T>& v)      const   { return x == v.x && y == v.y && z == v.z && w == v.w; }
        bool operator!=(const Vec4<T>& v)      const   { return !(*this == v); }

        T& operator[](int i)
        {
            assert(i >= 0 && i < 4);

            switch (i)
            {
                case 0:
                    return x;
                case 1:
                    return y;
                case 2:
                    return z;
                default:
                    return w;
            }
        }
        const T& operator[](int i) const
        {
            assert(i >= 0 && i < 4);

            switch (i)
            {
                case 0:
                    return x;
                case 1:
                    return y;
                case 2:
                    return z;
                default:
                    return w;
            }
        }


        template<typename R>
        Vec4<R> to() const { return Vec4<R>{ static_cast<R>(x), static_cast<R>(y), static_cast<R>(z), static_cast<R>(w) }; }

        Vec3<T> toEuclidean() const
        {
            if (w != T{ 0 })
                return Vec3<T>{ x / w, y / w, z / w };

            return Vec3<T>{ x, y, z };
        }

        Vec3<T> toVec3() const
        {
            return Vec3<T>{ x, y, z };
        }

        vec2<T> tovec2() const
        {
            return vec2<T>{ x, y };
        }

        Vec4<T> operator-()                     const   { return Vec4<T>{ -x, -y, -z, -w }; }
        Vec4<T> operator+(const Vec4<T>& v)     const   { return Vec4<T>{ x + v.x, y + v.y, z + v.z, w + v.w }; }
        Vec4<T> operator-(const Vec4<T>& v)     const   { return Vec4<T>{ x - v.x, y - v.y, z - v.z, w - v.w }; }
        Vec4<T> operator*(const Vec4<T>& v)     const   { return Vec4<T>{ x * v.x, y * v.y, z * v.z, w * v.w }; }
        Vec4<T> operator/(const Vec4<T>& v)     const   { return Vec4<T>{ x / v.x, y / v.y, z / v.z, w / v.w }; }
        Vec4<T> operator+(T c)                  const   { return Vec4<T>{ x + c, y + c, z + c, w + c }; }
        Vec4<T> operator-(T c)                  const   { return Vec4<T>{ x - c, y - c, z - c, w - c }; }
        Vec4<T> operator*(T c)                  const   { return Vec4<T>{ x * c, y * c, z * c, w * c }; }
        Vec4<T> operator/(T c)                  const   { return Vec4<T>{ x / c, y / c, z / c, w / c }; }
        Vec4<T>& operator+=(const Vec4<T>& v)           { return *this = *this + v; }
        Vec4<T>& operator-=(const Vec4<T>& v)           { return *this = *this - v; }
        Vec4<T>& operator*=(const Vec4<T>& v)           { return *this = *this * v; }
        Vec4<T>& operator/=(const Vec4<T>& v)           { return *this = *this / v; }
        Vec4<T>& operator+=(T c)                        { return *this = *this + c; }
        Vec4<T>& operator-=(T c)                        { return *this = *this - c; }
        Vec4<T>& operator*=(T c)                        { return *this = *this * c; }
        Vec4<T>& operator/=(T c)                        { return *this = *this / c; }

        static T dot(const Vec4<T>& u, const Vec4<T>& v)
        {
            return static_cast<T>(u.x * v.x + u.y * v.y + u.z * v.z + u.w * v.w);
        }

        static Vec4<T> normalize(const Vec4<T>& v)
        {
            if (T length = v.l2norm(); length > T{ 0 })
                return Vec4<T>{ v.x / length, v.y / length, v.z / length, v.w / length };
            return v;
        }

        static T manhattanDistance(const Vec4<T>& u, const Vec4<T>& v)
        {
            return static_cast<T>((u - v).l1norm());
        }

        static T euclideanDistance(const Vec4<T>& u, const Vec4<T>& v)
        {
            return static_cast<T>((u - v).l2norm());
        }

        T l1norm() const
        {
            return static_cast<T>(std::abs(x) + std::abs(y) + std::abs(z) + std::abs(w));
        }

        T l2norm() const
        {
            return static_cast<T>(std::sqrt(dot(*this)));
        }

        T dot(const Vec4<T>& v)                const { return dot(*this, v); }
        Vec4<T> normalize()                    const { return normalize(*this); }
        T manhattanDistance(const Vec4<T>& v)  const { return manhattanDistance(*this, v); }
        T euclideanDistance(const Vec4<T>& v)  const { return euclideanDistance(*this, v); }

        Vec4<T> floor() const
        {
            return Vec4<T>{ std::floor(x), std::floor(y), std::floor(z), std::floor(w) };
        }

        Vec4<T> ceil() const
        {
            return Vec4<T>{ std::ceil(x), std::ceil(y), std::ceil(z), std::ceil(w) };
        }

        Vec4<T> abs() const
        {
            return Vec4<T>{ std::abs(x), std::abs(y), std::abs(z), std::abs(w) };
        }
    };

    template<typename T> Vec4<T> operator+(T c, const Vec4<T>& v) { return v + c; }
    template<typename T> Vec4<T> operator-(T c, const Vec4<T>& v) { return Vec4<T>{ c - v.x, c - v.y, c - v.z, c - v.w }; }
    template<typename T> Vec4<T> operator*(T c, const Vec4<T>& v) { return v * c; }
    template<typename T> Vec4<T> operator/(T c, const Vec4<T>& v) { return Vec4<T>{ c / v.x, c / v.y, c / v.z, c / v.w }; }

    using double4 = Vec4<double>;
    using float4 = Vec4<float>;
    using int4 = Vec4<int>;
    using uint4 = Vec4<unsigned int>;
    using char4 = Vec4<char>;
    using uchar4 = Vec4<unsigned char>;
    using size_t4 = Vec4<size_t>;
}
