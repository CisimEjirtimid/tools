#pragma once
#include "meta.h"
#include <utility>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <ctime>

#define NOT_A_MACRO

// Example:
// auto ts = mdsp::Time::FromDShowUnits(330000);
// ts += std::chrono::milliseconds(33);
// auto millis = ts.milliseconds<int64_t>(); // Will result in 66ms 

namespace mdsp
{
    // Generic Time class with convenience methods for conversions between multiple time units
    // Constructed with explicit std::chrono::duration or named constructors From{TIME_UNIT}
    class Time
    {
    public:
        using PeriodMinute = std::ratio<60>;
        using PeriodHour = std::ratio_multiply<std::ratio<60>, PeriodMinute>;
        using PeriodDay = std::ratio_multiply<std::ratio<24>, PeriodHour>;
        using PeriodSecond = std::ratio<1, 1>;
        using PeriodMillisecond = std::milli;
        using PeriodMicrosecond = std::micro;
        using PeriodNanosecond = std::nano;
        using PeriodDShow = std::ratio<100, 1000'000'000>;
        using PeriodPTS = std::ratio<1, 90'000>;

        template<typename T, typename Period>
        using Duration = std::chrono::duration<T, Period>;

        using Representation = Duration<int64_t, std::ratio<1, 90'000'000>>;

        // Prints out Time to string in following way:
        // - If the Time is greater than an hour, the output is hh hrs mm min (e.g. '13 hrs 55 min')
        // - If the Time is not greater than hour but it's greater than a minute, the output is mm min ss sec (e.g. '55 min 12 sec')
        // - If neither of the above is true, the output is just ss sec (e.g. '12 sec')
        // It's suitable for printing duration between two points in time
        std::string toStringFuzzy() const
        {
            double secs = this->seconds<double>();
            int mins = int(secs) / 60;
            int hours = mins / 60;

            std::stringstream out;

            if (hours >= 1)
            {
                double mins2 = mins - 60 * hours;
                out << hours << " hrs";

                if (mins2 > 0)
                    out << " " << mins2 << " min";
            }
            else if (mins >= 1)
            {
                int secs2 = int(secs) - 60 * mins;
                out << mins << " min";

                if (secs2 > 0)
                    out << " " << secs2 << " sec";
            }
            else
            {
                out << int(secs) << " sec";
            }

            return out.str();
        }

    protected:
        // Compiler will choose this overload if internal Representation is losslessly convertible to the provided std::chrono::duration
        template<typename T, typename Enable = std::enable_if_t<meta::IsConstructibleWith<T, Representation>::value>, typename Dummy = Enable>
        constexpr T to() const
        {
            return T(_value);
        }

        // Compiler will choose this overload if internal Representation is NOT losslessly convertible to the provided std::chrono::duration
        template<typename T, typename Enable = std::enable_if_t<!meta::IsConstructibleWith<T, Representation>::value>>
        constexpr T to() const
        {
            return std::chrono::duration_cast<T>(_value);
        }

        Representation _value;

    public:
        constexpr Time() noexcept
            : _value(0)
        {
        }

        // Compiler will choose this overload if provided std::chrono::duration is losslessly convertible to the internal Representation
        template<typename T, typename Period, typename Enable = std::enable_if_t<meta::IsConstructibleWith<Representation, Duration<T, Period>>::value>, typename Dummy = Enable>
        constexpr Time(Duration<T, Period> duration)
            : _value(std::move(duration))
        {
        }

        // Compiler will choose this overload if provided std::chrono::duration is NOT losslessly convertible to the internal Representation
        template<typename T, typename Period, typename Enable = std::enable_if_t<!meta::IsConstructibleWith<Representation, Duration<T, Period>>::value>>
        constexpr Time(Duration<T, Period> duration)
            : _value(std::chrono::duration_cast<Representation>(std::move(duration)))
        {
        }
        
        template<typename T>
        static constexpr Time FromDays(T value)
        {
            return Time{ Duration<T, PeriodDay>(value) };
        }

        template<typename T>
        static constexpr Time FromHours(T value)
        {
            return Time{ Duration<T, PeriodHour>(value) };
        }

        template<typename T>
        static constexpr Time FromMinutes(T value)
        {
            return Time{ Duration<T, PeriodMinute>(value) };
        }

        template<typename T>
        static constexpr Time FromSeconds(T value)
        {
            return Time{ Duration<T, PeriodSecond>(value) };
        }

        template<typename T>
        static constexpr Time FromMilliseconds(T value)
        {
            return Time{ Duration<T, PeriodMillisecond>(value) };
        }

        template<typename T>
        static constexpr Time FromMicroseconds(T value)
        {
            return Time{ Duration<T, PeriodMicrosecond>(value) };
        }

        template<typename T>
        static constexpr Time FromNanoseconds(T value)
        {
            return Time{ Duration<T, PeriodNanosecond>(value) };
        }

        template<typename T>
        static constexpr Time FromPTSUnits(T value)
        {
            return Time{ Duration<T, PeriodPTS>(value) };
        }

        template<typename T>
        static constexpr Time FromDShowUnits(T value)
        {
            return Time{ Duration<T, PeriodDShow>(value) };
        }

        template<typename T>
        static constexpr Time FromRepr(T value)
        {
            return Time{ Representation(value) };
        }

        // static_assert has to be used because of C++/CLI
        template<typename T>
        static constexpr Time FromNTP(T value)
        {
            using namespace std::chrono_literals;

            using PeriodNTPFraction = std::ratio<1, 0x100000000>;
            using NTPFraction = Duration<std::int64_t, PeriodNTPFraction>;

            static_assert(std::is_integral_v<T>, "Specified template parameter is not an integral type");

            constexpr auto UNIX_EPOCH = std::chrono::sys_days{ 1970y / 1 / 1 };
            constexpr auto NTP_EPOCH = std::chrono::sys_days{ 1900y / 1 / 1 };
            constexpr auto EPOCH_DIFF = Time(UNIX_EPOCH - NTP_EPOCH);

            auto seconds = Time::FromSeconds(value >> 32);
            auto fractions = Time(NTPFraction{ value & 0xffffffff });

            return (seconds - EPOCH_DIFF) + fractions;
        }

        // static_assert has to be used because of C++/CLI
        template<typename T, typename U>
        static constexpr Time FromRational(T value, U num, U den)
        {
            using Period = typename Representation::period;

            static_assert(std::is_arithmetic<meta::raw<T>>::value,
                "value must be an arithmetic type");
            static_assert(std::is_arithmetic<meta::raw<U>>::value,
                "num and den must be arithmetic types");

            assert(den > 0);

            return Time { Representation(Period::den / Period::num * value * num / den) };
        }

        static Time Max()
        {
            return FromDShowUnits((std::numeric_limits<int64_t>::max)());
        }

        static Time Now()
        {
            return Time{ std::chrono::system_clock::now().time_since_epoch() };
        }

        static Time NowHighRes()
        {
            return Time{ std::chrono::high_resolution_clock::now().time_since_epoch() };
        }

        static constexpr Time Zero()
        {
            return Time{ Representation(0) };
        }

        constexpr Time(const Time&) = default;
        constexpr Time(Time&&) noexcept = default;

        Time& operator=(const Time&) = default;
        Time& operator=(Time&&) noexcept = default;

        // Convenience functions
        template<typename T = int64_t>
        constexpr auto chronoDays() const
        {
            return to<Duration<T, PeriodDay>>();
        }

        template<typename T>
        constexpr T days() const
        {
            return chronoDays<T>().count();
        }

        template<typename T = int64_t>
        constexpr auto chronoHours() const
        {
            return to<Duration<T, PeriodHour>>();
        }

        template<typename T>
        constexpr T hours() const
        {
            return chronoHours<T>().count();
        }

        template<typename T = int64_t>
        constexpr auto chronoMinutes() const
        {
            return to<Duration<T, PeriodMinute>>();
        };

        template<typename T>
        constexpr T minutes() const
        {
            return chronoMinutes<T>().count();
        };

        template<typename T = int64_t>
        constexpr auto chronoSeconds() const
        {
            return to<Duration<T, PeriodSecond>>();
        }

        template<typename T>
        constexpr T seconds() const
        {
            return chronoSeconds<T>().count();
        }

        template<typename T = int64_t>
        constexpr auto chronoMilliseconds() const
        {
            return to<Duration<T, PeriodMillisecond>>();
        }

        template<typename T>
        constexpr T milliseconds() const
        {
            return chronoMilliseconds<T>().count();
        }

        template<typename T = int64_t>
        constexpr auto chronoMicroseconds() const
        {
            return to<Duration<T, PeriodMicrosecond>>();
        }

        template<typename T>
        constexpr T microseconds() const
        {
            return chronoMicroseconds<T>().count();
        }

        template<typename T = int64_t>
        constexpr auto chronoNanoseconds() const
        {
            return to<Duration<T, PeriodNanosecond>>();
        }

        template<typename T>
        constexpr T nanoseconds() const
        {
            return chronoNanoseconds<T>().count();
        }

        template<typename T = int64_t>
        constexpr auto chronoDirectShowUnits() const
        {
            return to<Duration<T, PeriodDShow>>();
        }

        template<typename T>
        constexpr T directShowUnits() const
        {
            return chronoDirectShowUnits<T>().count();
        }

        template<typename T = int64_t>
        constexpr auto chronoPTSUnits() const
        {
            return to<Duration<T, PeriodPTS>>();
        }

        template<typename T>
        constexpr T PTSUnits() const
        {
            return chronoPTSUnits<T>().count();
        }

        template<typename T = int64_t>
        constexpr auto chronoRepr() const
        {
            return to<Duration<T, typename Representation::period>>();
        }

        template<typename T>
        constexpr T repr() const
        {
            return chronoRepr<T>().count();
        }

        // Various operations

        constexpr bool inRange(const Time& left, const Time& right) const
        {
            return _value >= left && _value <= right;
        }

        static constexpr Time min NOT_A_MACRO(const Time& lhs, const Time& rhs)
        {
            if (lhs < rhs)
                return lhs;

            return rhs;
        }

        static constexpr Time max NOT_A_MACRO(const Time& lhs, const Time& rhs)
        {
            if (lhs > rhs)
                return lhs;

            return rhs;
        }

        static constexpr Time abs(Time t)
        {
            if (t > Time::Zero())
                return t;

            return Time::Zero() - t;
        }

        // Arithmethic operators

        constexpr Time operator-() const
        {
            return Time(-_value);
        }

        constexpr Time& operator++()
        {
            ++_value;

            return *this;
        }

        constexpr Time& operator--()
        {
            --_value;

            return *this;
        }

        constexpr Time& operator+=(const Time& rhs)
        {
            _value += rhs._value;

            return *this;
        }

        constexpr Time& operator-=(const Time& rhs)
        {
            _value -= rhs._value;

            return *this;
        }

        // static_assert has to be used because of C++/CLI
        template<typename T>
        constexpr Time& operator*=(T&& rhs)
        {
            static_assert(std::is_arithmetic_v<meta::raw<T>>,
                "Specified template argument is not an arithmetic type");

            _value *= rhs;
            return *this;
        }

        // static_assert has to be used because of C++/CLI
        template<typename T>
        constexpr Time& operator/=(T&& rhs)
        {
            static_assert(std::is_arithmetic_v<meta::raw<T>>,
                "Specified template argument is not an arithmetic type");

            _value /= rhs;
            return *this;
        }

        constexpr friend Time operator+(const Time& lhs, const Time& rhs)
        {
            return Time(lhs._value + rhs._value);
        }

        constexpr friend Time operator-(const Time& lhs, const Time& rhs)
        {
            return Time(lhs._value - rhs._value);
        }

        // static_assert has to be used because of C++/CLI
        template<typename T>
        constexpr friend Time operator*(T&& lhs, const Time& rhs)
        {
            static_assert(std::is_arithmetic_v<meta::raw<T>>,
                "Specified template argument is not an arithmetic type");

            return Time(lhs * rhs._value);
        }

        // static_assert has to be used because of C++/CLI
        template<typename T>
        constexpr friend Time operator*(const Time& lhs, T&& rhs)
        {
            static_assert(std::is_arithmetic_v<meta::raw<T>>,
                "Specified template argument is not an arithmetic type");

            return Time(lhs._value * rhs);
        }

        // static_assert has to be used because of C++/CLI
        template<typename T, typename Enable = typename std::enable_if_t<!meta::IsSame<Time, meta::raw<T>>::value>>
        constexpr friend Time operator/(const Time& lhs, T&& rhs)
        {
            static_assert(std::is_arithmetic_v<meta::raw<T>>,
                "Specified template argument is not an arithmetic type");

            return Time(lhs._value / rhs);
        }

        constexpr friend double operator/(const Time& lhs, const Time& rhs)
        {
            return lhs.directShowUnits<double>() / rhs.directShowUnits<double>();
        }

        // Relational operators

        constexpr friend bool operator==(const Time& lhs, const Time& rhs)
        {
            return lhs._value == rhs._value;
        }

        constexpr friend bool operator!=(const Time& lhs, const Time& rhs)
        {
            return !(lhs._value == rhs._value);
        }

        constexpr friend bool operator<(const Time& lhs, const Time& rhs)
        {
            return lhs._value < rhs._value;
        }

        constexpr friend bool operator>(const Time& lhs, const Time& rhs)
        {
            return rhs._value < lhs._value;
        }

        constexpr friend bool operator<=(const Time& lhs, const Time& rhs)
        {
            return !(lhs._value > rhs._value);
        }

        constexpr friend bool operator>=(const Time& lhs, const Time& rhs)
        {
            return !(lhs._value < rhs._value);
        }
    };

    // User defined literals

    constexpr Time operator "" _s(unsigned long long value)
    {
        return Time::FromSeconds(value);
    }

    constexpr Time operator "" _s(long double value)
    {
        return Time::FromSeconds(value);
    }

    constexpr Time operator "" _ms(unsigned long long value)
    {
        return Time::FromMilliseconds(value);
    }

    constexpr Time operator "" _ms(long double value)
    {
        return Time::FromMilliseconds(value);
    }

    constexpr Time operator "" _us(unsigned long long value)
    {
        return Time::FromMicroseconds(value);
    }

    constexpr Time operator "" _us(long double value)
    {
        return Time::FromMicroseconds(value);
    }

    constexpr Time operator "" _ns(unsigned long long value)
    {
        return Time::FromNanoseconds(value);
    }

    constexpr Time operator "" _ns(long double value)
    {
        return Time::FromNanoseconds(value);
    }

    constexpr Time operator "" _m(unsigned long long value)
    {
        return Time::FromMinutes(value);
    }

    constexpr Time operator "" _m(long double value)
    {
        return Time::FromMinutes(value);
    }

    constexpr Time operator "" _h(unsigned long long value)
    {
        return Time::FromHours(value);
    }

    constexpr Time operator "" _h(long double value)
    {
        return Time::FromHours(value);
    }

    constexpr Time operator "" _tick(unsigned long long value)
    {
        return Time::FromDShowUnits(value);
    }

    constexpr Time operator "" _tick(long double value)
    {
        return Time::FromDShowUnits(value);
    }

    constexpr Time operator "" _pts(unsigned long long value)
    {
        return Time::FromPTSUnits(value);
    }

    constexpr Time operator "" _pts(long double value)
    {
        return Time::FromPTSUnits(value);
    }

    static constexpr Time TS_NOT_INIT = Time::FromSeconds(-5);
}

namespace std
{
    template<>
    struct hash<mdsp::Time>
    {
        size_t operator()(const mdsp::Time& time) const
        {
            return std::hash<int64_t>{}(time.microseconds<int64_t>());
        }
    };
}
