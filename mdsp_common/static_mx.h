#pragma once
#include "static_vec.h"
#include <assert.h>
#include <type_traits>

namespace mdsp
{
    // TODO: think about row-wise implementation because of operator[]
    #define FLOAT_DOUBLE_COMPARE_EPSILON 1e-9

    template<typename T, int rows, int cols>
    struct Mx
    {
        Mx() = default;
        explicit Mx(const T arr[rows][cols]);
        explicit Mx(const T arr[rows * cols]);

        // static_assert has to be used because of C++/CLI
        template<typename U, typename Enable = std::enable_if_t<std::is_same_v<T, U>>, typename... Ts>
        constexpr explicit Mx(U first, Ts... rest)
            : _data{ first, rest... }
        {
            static_assert(sizeof...(Ts) + 1 == rows * cols,
                "Number of template arguments specified is not equal to the number of matrix elements");
        }

        Mx<T, rows, cols>& operator=(const T arr[rows][cols]);
        Mx<T, rows, cols>& operator=(const T arr[rows * cols]);

        int height() const { return rows; }
        int width()  const { return cols; }
        int pitch()  const { return rows; }

        // safe in debug
              T& operator() (int i, int j)       { assert(i < rows && j < cols); return _data[i * cols + j]; }
        const T& operator() (int i, int j) const { assert(i < rows && j < cols); return _data[i * cols + j]; }

        // safe in debug
              T& operator[] (int i)       { assert(i < rows * cols); return _data[i]; }
        const T& operator[] (int i) const { assert(i < rows * cols); return _data[i]; }

        template<typename R, int m, int k, int n>
        friend Mx<R, m, n>& mul(Mx<R, m, n>& C, const Mx<R, m, k>& A, const Mx<R, k, n>& B);

        static Mx<T, rows, cols>& add(Mx<T, rows, cols>& C, const Mx<T, rows, cols>& A, const Mx<T, rows, cols>& B);
        static Mx<T, rows, cols>& sub(Mx<T, rows, cols>& C, const Mx<T, rows, cols>& A, const Mx<T, rows, cols>& B);
        static Mx<T, cols, rows>& transpose(Mx<T, cols, rows>& A, const Mx<T, rows, cols>& B);

        template<typename R, int m>
        friend Mx<R, m, m>& transpose(Mx<R, m, m>& A);

        static Mx<T, rows, cols>& addc(Mx<T, rows, cols>& A, T c);
        static Mx<T, rows, cols>& mulc(Mx<T, rows, cols>& A, T c);
        static Mx<T, rows, cols>& divc(Mx<T, rows, cols>& A, T c);
        static Mx<T, rows, cols>& mulx(Mx<T, rows, cols>& A, const Mx<T, rows, cols>& B);

        template<typename R, int m, int k, int n>
        friend Mx<R, m, n> operator*(const Mx<R, m, k>& A, const Mx<R, k, n>& B);

        template<typename R, int m>
        friend vec2<R> operator*(const Mx<R, m, 2>& A, const vec2<R>& v);

        template<typename R, int m>
        friend Vec3<R> operator*(const Mx<R, m, 3>& A, const Vec3<R>& v);

        template<typename R, int m>
        friend Vec4<R> operator*(const Mx<R, m, 4>& A, const Vec4<R>& v);

        Mx<T, rows, cols>   operator-(const Mx<T, rows, cols>& B) const;
        Mx<T, rows, cols>   operator+(const Mx<T, rows, cols>& B) const;

        template<typename R, int rows1, int cols1, int rows2, int cols2>
        friend bool operator==(const Mx<R, rows1, cols1>& A, const Mx<R, rows2, cols2>& B);

        Mx<T, rows, cols> operator*(T c) const;
        Mx<T, rows, cols> operator/(T c) const;
        Mx<T, rows, cols> operator-(T c) const;
        Mx<T, rows, cols> operator+(T c) const;

        Mx<T, cols, rows> transpose() const;
        bool isEye() const;

        T* data() { return _data; }
        const T* data() const { return _data; }

    protected:
        T _data[rows * cols];
    };

    ///////////////////////////////////////////////////////////////////////////////////////
    // helper functions

    template<typename T>
    bool differenceGreaterThanFloatDoubleEps(T a, T b)
    {
        return abs(a - b) > FLOAT_DOUBLE_COMPARE_EPSILON;
    }

    template<typename T, int rows, int cols>
    void zeros(Mx<T, rows, cols>& A)
    {
        for (int i = 0; i < rows * cols; i++)
            A[i] = T{ 0 };
    }

    template<typename T, int rows, int cols>
    void eye(Mx<T, rows, cols>& A)
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
                A(i, j) = (i == j) ? T{ 1 } : T{ 0 };
        }
    }

    template<typename T, int rows, int cols>
    void fill(Mx<T, rows, cols>& A, T value)
    {
        for (int i = 0; i < rows * cols; i++)
            A[i] = value;
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    template<typename T, int rows, int cols>
    Mx<T, rows, cols>::Mx(const T arr[rows][cols])
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
                _data[i * cols + j] = arr[i][j];
        }
    }

    template<typename T, int rows, int cols>
    Mx<T, rows, cols>::Mx(const T arr[rows * cols])
    {
        for (int i = 0; i < rows * cols; i++)
            _data[i] = arr[i];
    }

    template<typename T, int rows, int cols>
    Mx<T, rows, cols>& Mx<T, rows, cols>::operator=(const T arr[rows][cols])
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
                _data[i * cols + j] = arr[i][j];
        }
       
        return *this;
    }

    template<typename T, int rows, int cols>
    Mx<T, rows, cols>& Mx<T, rows, cols>::operator=(const T arr[rows * cols])
    {
        for (int i = 0; i < rows * cols; i++)
            _data[i] = arr[i];

        return *this;
    }

    template<typename T, int m, int k, int n>
    Mx<T, m, n>& mul(Mx<T, m, n>& C, const Mx<T, m, k>& A, const Mx<T, k, n>& B)
    {
        // keep in mind - internal dimension k must agree (m x k) * (k x n)
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
            {
                T acc{ 0 };

                for (int l = 0; l < k; l++)
                    acc += A(i, l) * B(l, j);

                C(i, j) = acc;
            }
        }

        return C;
    }

    template<typename T, int rows, int cols>
    Mx<T, rows, cols>& Mx<T, rows, cols>::add(Mx<T, rows, cols>& C, const Mx<T, rows, cols>& A, const Mx<T, rows, cols>& B)
    {
        for (int i = 0; i < rows * cols; i++)
            C._data[i] = A._data[i] + B._data[i];

        return C;
    }

    template<typename T, int rows, int cols>
    Mx<T, rows, cols>& Mx<T, rows, cols>::sub(Mx<T, rows, cols>& C, const Mx<T, rows, cols>& A, const Mx<T, rows, cols>& B)
    {
        for (int i = 0; i < rows * cols; i++)
            C._data[i] = A._data[i] - B._data[i];

        return C;
    }

    template<typename T, int m>
    Mx<T, m, m>& transpose(Mx<T, m, m>& A)
    {
        for (int i = 0; i < m; i++)
        {
            for (int j = i + 1; j < m; j++)
            {
                T tmp  = A(i, j);
                A(i, j) = A(j, i);
                A(j, i) = tmp;
            }
        }

        return A;
    }

    template<typename T, int rows, int cols>
    Mx<T, cols, rows>& Mx<T, rows, cols>::transpose(Mx<T, cols, rows>& A, const Mx<T, rows, cols>& B)
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
                A(j, i) = B(i, j);
        }

        return A;
    }

    template<typename T, int rows, int cols>
    Mx<T, rows, cols>& Mx<T, rows, cols>::addc(Mx<T, rows, cols>& A, T c)
    {
        for (int i = 0; i < rows * cols; i++)
            A._data[i] += c;

        return A;
    }

    template<typename T, int rows, int cols>
    Mx<T, rows, cols>& Mx<T, rows, cols>::mulc(Mx<T, rows, cols>& A, T c)
    {
        for (int i = 0; i < rows * cols; i++)
            A._data[i] *= c;

        return A;
    }

    template<typename T, int rows, int cols>
    Mx<T, rows, cols>& Mx<T, rows, cols>::divc(Mx<T, rows, cols>& A, T c)
    {
        for (int i = 0; i < rows * cols; i++)
            A._data[i] /= c;

        return A;
    }

    template<typename T, int rows, int cols>
    Mx<T, rows, cols>& Mx<T, rows, cols>::mulx(Mx<T, rows, cols>& A, const Mx<T, rows, cols>& B)
    {
        for (int i = 0; i < rows * cols; i++)
            A._data[i] *= B._data[i];

        return A;
    }

    template<typename T, int m, int k, int n>
    Mx<T, m, n> operator*(const Mx<T, m, k>& A, const Mx<T, k, n>& B)
    {
        Mx<T, m, n> C;
        mul<T, m, k, n>(C, A, B);
        return C;
    }

    template<typename T>
    vec2<T> operator*(const Mx<T, 2, 2>& A, const vec2<T>& v)
    {
        vec2<T> r;

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
                r[i] += A(i, j) * v[j];
        }

        return r;
    }

    template<typename T>
    Vec3<T> operator*(const Mx<T, 3, 3>& A, const Vec3<T>& v)
    {
        Vec3<T> r;

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
                r[i] += A(i, j) * v[j];
        }

        return r;
    }

    template<typename T>
    Vec4<T> operator*(const Mx<T, 4, 4>& A, const Vec4<T>& v)
    {
        Vec4<T> r;
        r.w = T{ 0 };

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
                r[i] += A(i, j) * v[j];
        }

        return r;
    }

    template<typename T, int rows, int cols>
    inline Mx<T, rows, cols> Mx<T, rows, cols>::operator-(const Mx<T, rows, cols>& B) const
    {
        Mx<T, rows, cols> C;
        Mx<T, rows, cols>::sub(C, *this, B);
        return C;
    }

    template<typename T, int rows, int cols>
    inline Mx<T, rows, cols> Mx<T, rows, cols>::operator+(const Mx<T, rows, cols>& B) const
    {
        Mx<T, rows, cols> C;
        Mx<T, rows, cols>::add(C, *this, B);
        return C;
    }

    template<typename T, int rows1, int cols1, int rows2, int cols2>
    bool operator==(const Mx<T, rows1, cols1>& A, const Mx<T, rows2, cols2>& B) 
    {
        if (rows1 != rows2 && cols1 != cols2)
            return false;

        for (int i = 0; i < rows1; i++)
        {
            for (int j = 0; j < cols1; j++)
            {
                if constexpr (std::is_floating_point<T>())
                {
                    if (differenceGreaterThanFloatDoubleEps(A(i, j), B(i, j)))
                        return false;
                }
                else
                {
                    if (A(i, j) != B(i, j))
                        return false;
                }
            }
        }
        return true;
    }

    template<typename T, int rows, int cols>
    inline Mx<T, rows, cols> Mx<T, rows, cols>::operator*(T c) const
    {
        Mx<T, rows, cols> A(*this);
        Mx<T, rows, cols>::mulc(A, c);
        return A;
    }

    template<typename T, int rows, int cols>
    inline Mx<T, rows, cols> Mx<T, rows, cols>::operator/(T c) const
    {
        Mx<T, rows, cols> A(*this);
        Mx<T, rows, cols>::divc(A, c);
        return A;
    }

    template<typename T, int rows, int cols>
    inline Mx<T, rows, cols> Mx<T, rows, cols>::operator-(T c) const
    {
        Mx<T, rows, cols> A(*this);
        Mx<T, rows, cols>::addc(A, -c);
        return A;
    }

    template<typename T, int rows, int cols>
    inline Mx<T, rows, cols> Mx<T, rows, cols>::operator+(T c) const
    {
        Mx<T, rows, cols> A(*this);
        Mx<T, rows, cols>::addc(A, c);
        return A;
    }

    template<typename T, int rows, int cols>
    inline Mx<T, cols, rows> Mx<T, rows, cols>::transpose() const
    {
        Mx<T, cols, rows> C;
        Mx<T, rows, cols>::transpose(C, *this);
        return C;
    }

    template<typename T, int rows, int cols>
    bool Mx<T, rows, cols>::isEye() const
    {
        bool isIt = true;

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (i == j && _data[i * cols + j] != T{ 1.0 })
                    isIt = false;

                if (i != j && _data[i * cols + j] != T{ 0.0 })
                    isIt = false;
            }
        }

        return isIt;
    }
}
