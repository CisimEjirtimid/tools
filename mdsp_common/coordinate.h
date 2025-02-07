#pragma once
#include "mdsp_types.h"
#include "geo_convert.h"

#undef max

namespace mdsp::spatial
{
    static constexpr double NaN = std::numeric_limits<double>::quiet_NaN();

    template<typename SRef>
    struct Coordinate;

    namespace sref
    {
        static constexpr double WGS84SemiMajorAxis = 6'378'137.0;
        static constexpr double WGS84SemiMinorAxis = 6'356'752.314'245;
        static constexpr double WebMercOrigin = 20'037'508.342'789'244;

        // actually EPSG:4979
        struct EPSG4326
        {
            struct Limits
            {
                static constexpr double X_MIN = -180.0;
                static constexpr double X_MAX = 180.0;

                static constexpr double Y_MIN = -90.0;
                static constexpr double Y_MAX = 90.0;
            };
        };

        struct RadiansEPSG4326
        {
            // TODO: implement using radians instead of degrees in EPSG4326 representation
        };

        struct QuasiEPSG4326
        {
            // TODO: implement using ECEFS representation as starting point,
            //       and taking arccos of X component [acos(ECEFS.x)] and arcsin of Z component [asin(ECEFS.z)]
            //       as a new coordinate representation
        };

        struct EPSG3857
        {
            struct Limits
            {
                static constexpr double X_MIN = -WebMercOrigin;
                static constexpr double X_MAX = WebMercOrigin;

                static constexpr double Y_MIN = -WebMercOrigin;
                static constexpr double Y_MAX = WebMercOrigin;
            };
        };

        struct ScaledEPSG3857
        {
            struct Limits
            {
                static constexpr double X_MIN = -1.0;
                static constexpr double X_MAX = 1.0;

                static constexpr double Y_MIN = -1.0;
                static constexpr double Y_MAX = 1.0;
            };
        };

        struct ECEF
        {
            struct Limits
            {
                static constexpr double X_MIN = -std::numeric_limits<double>::max();
                static constexpr double X_MAX = std::numeric_limits<double>::max();

                static constexpr double Y_MIN = -std::numeric_limits<double>::max();
                static constexpr double Y_MAX = std::numeric_limits<double>::max();

                static constexpr double Z_MIN = -std::numeric_limits<double>::max();
                static constexpr double Z_MAX = std::numeric_limits<double>::max();
            };
        };

        struct ScaledECEF
        {
            struct Limits
            {
                static constexpr double X_MIN = ECEF::Limits::X_MIN / WGS84SemiMajorAxis;
                static constexpr double X_MAX = ECEF::Limits::X_MAX / WGS84SemiMajorAxis;

                static constexpr double Y_MIN = ECEF::Limits::Y_MIN / WGS84SemiMajorAxis;
                static constexpr double Y_MAX = ECEF::Limits::Y_MAX / WGS84SemiMajorAxis;

                static constexpr double Z_MIN = ECEF::Limits::Z_MIN / WGS84SemiMajorAxis;
                static constexpr double Z_MAX = ECEF::Limits::Z_MAX / WGS84SemiMajorAxis;
            };
        };

        namespace detail
        {
            template<typename SRef>
            inline void wrap(double& x, double& y)
            {
                auto vertical_wrap =
                    [](double& col, double& row, double lo, double hi, double horizontal_range)
                {
                    double vertical_range = (hi - lo) * 2.0;

                    double half_horizontal = horizontal_range / 2.0;

                    row = std::fmod(row, vertical_range);

                    while (row < lo || row > hi)
                    {
                        if (row < lo)
                        {
                            row = lo + (lo - row);
                            col += half_horizontal;
                        }

                        if (row > hi)
                        {
                            row = hi - (row - hi);
                            col += half_horizontal;
                        }
                    }
                };

                auto horizontal_wrap = [](double& col, double lo, double hi)
                {
                    double range = hi - lo;

                    col = std::fmod(col, range);

                    while (col < lo)
                        col += range;

                    while (col > hi)
                        col -= range;
                };

                auto x_min = SRef::Limits::X_MIN;
                auto x_max = SRef::Limits::X_MAX;

                auto y_min = SRef::Limits::Y_MIN;
                auto y_max = SRef::Limits::Y_MAX;

                vertical_wrap(x, y, y_min, y_max, x_max - x_min);
                horizontal_wrap(x, x_min, x_max);
            }

            template<typename From, typename To>
            inline void convert(double& x, double& y, double& z);

            template<>
            inline void convert<EPSG4326, EPSG3857>(double& x, double& y, [[maybe_unused]] double& z)
            {
                if (abs(x) > EPSG4326::Limits::X_MAX || abs(y) > EPSG4326::Limits::Y_MAX)
                {

                    x = NaN;
                    y = NaN;
                    return;
                }

                // clamp to display-able coordinate
                // as last 5 degrees of latitude are without data
                y = std::clamp(y, -85.05, 85.05);

                auto num = x * 0.017453292519943295;
                auto a = y * 0.017453292519943295;

                x = 6378137.0 * num;
                y = 3189068.5 * log((1.0 + sin(a)) / (1.0 - sin(a)));
            }

            template<>
            inline void convert<EPSG3857, EPSG4326>(double& x, double& y, [[maybe_unused]] double& z)
            {
                if ((abs(x) > EPSG3857::Limits::X_MAX) || (abs(y) > EPSG3857::Limits::Y_MAX))
                {
                    x = NaN;
                    y = NaN;

                    return;
                }

                auto X = x / WGS84SemiMajorAxis;
                auto Y = 1.5707963267948966 - (2.0 * atan(exp((-1.0 * y) / WGS84SemiMajorAxis)));

                x = math::rad2deg(X);
                y = math::rad2deg(Y);
            }

            template<>
            inline void convert<EPSG3857, ScaledEPSG3857>(double& x, double& y, double& z)
            {
                x /= WebMercOrigin;
                y /= WebMercOrigin;
                z /= WebMercOrigin;
            }

            template<>
            inline void convert<ScaledEPSG3857, EPSG3857>(double& x, double& y, double& z)
            {
                x *= WebMercOrigin;
                y *= WebMercOrigin;
                z *= WebMercOrigin;
            }

            template<>
            inline void convert<EPSG4326, ECEF>(double& x, double& y, double& z)
            {
                auto result = convertGeodeticToECEF(double3{ y, x, z });
                std::tie(x, y, z) = std::tie(result.x, result.y, result.z);
            }

            template<>
            inline void convert<ECEF, EPSG4326>(double& x, double& y, double& z)
            {
                auto result = convertECEFToGeodetic(double3{ x, y, z });
                std::tie(y, x, z) = std::tie(result.x, result.y, result.z);
            }

            template<>
            inline void convert<ECEF, ScaledECEF>(double& x, double& y, double& z)
            {
                x /= WGS84SemiMajorAxis;
                y /= WGS84SemiMajorAxis;
                z /= WGS84SemiMinorAxis;
            }

            template<>
            inline void convert<ScaledECEF, ECEF>(double& x, double& y, double& z)
            {
                x *= WGS84SemiMajorAxis;
                y *= WGS84SemiMajorAxis;
                z *= WGS84SemiMinorAxis;
            }

            template<>
            inline void convert<EPSG4326, ScaledEPSG3857>(double& x, double& y, double& z)
            {
                convert<EPSG4326, EPSG3857>(x, y, z);
                convert<EPSG3857, ScaledEPSG3857>(x, y, z);
            }

            template<>
            inline void convert<EPSG4326, ScaledECEF>(double& x, double& y, double& z)
            {
                convert<EPSG4326, ECEF>(x, y, z);
                convert<ECEF, ScaledECEF>(x, y, z);
            }

            template<>
            inline void convert<EPSG3857, ECEF>(double& x, double& y, double& z)
            {
                convert<EPSG3857, EPSG4326>(x, y, z);
                convert<EPSG4326, ECEF>(x, y, z);
            }

            template<>
            inline void convert<EPSG3857, ScaledECEF>(double& x, double& y, double& z)
            {
                convert<EPSG3857, ECEF>(x, y, z);
                convert<ECEF, ScaledECEF>(x, y, z);
            }

            template<>
            inline void convert<ScaledEPSG3857, EPSG4326>(double& x, double& y, double& z)
            {
                convert<ScaledEPSG3857, EPSG3857>(x, y, z);
                convert<EPSG3857, EPSG4326>(x, y, z);
            }

            template<>
            inline void convert<ScaledEPSG3857, ECEF>(double& x, double& y, double& z)
            {
                convert<ScaledEPSG3857, EPSG3857>(x, y, z);
                convert<EPSG3857, ECEF>(x, y, z);
            }

            template<>
            inline void convert<ScaledEPSG3857, ScaledECEF>(double& x, double& y, double& z)
            {
                convert<ScaledEPSG3857, ECEF>(x, y, z);
                convert<ECEF, ScaledECEF>(x, y, z);
            }

            template<>
            inline void convert<ECEF, EPSG3857>(double& x, double& y, double& z)
            {
                convert<ECEF, EPSG4326>(x, y, z);
                convert<EPSG4326, EPSG3857>(x, y, z);
            }

            template<>
            inline void convert<ECEF, ScaledEPSG3857>(double& x, double& y, double& z)
            {
                convert<ECEF, EPSG3857>(x, y, z);
                convert<EPSG3857, ScaledEPSG3857>(x, y, z);
            }

            template<>
            inline void convert<ScaledECEF, EPSG4326>(double& x, double& y, double& z)
            {
                convert<ScaledECEF, ECEF>(x, y, z);
                convert<ECEF, EPSG4326>(x, y, z);
            }

            template<>
            inline void convert<ScaledECEF, EPSG3857>(double& x, double& y, double& z)
            {
                convert<ScaledECEF, EPSG4326>(x, y, z);
                convert<EPSG4326, EPSG3857>(x, y, z);
            }

            template<>
            inline void convert<ScaledECEF, ScaledEPSG3857>(double& x, double& y, double& z)
            {
                convert<ScaledECEF, EPSG3857>(x, y, z);
                convert<EPSG3857, ScaledEPSG3857>(x, y, z);
            }
        }
    }

    template<typename SRef>
    struct Coordinate : double3
    {
        Coordinate()
            : double3(NaN, NaN, 0.0)
        {
        }

        Coordinate(double x, double y, double z = 0.0)
            : double3(x, y, z)
        {
            sref::detail::wrap<SRef>(this->x, this->y);
        }

        Coordinate(const double2& point)
            : double3(point, 0.0)
        {
            sref::detail::wrap<SRef>(this->x, this->y);
        }

        Coordinate(const double3& point)
            : double3(point)
        {
            sref::detail::wrap<SRef>(this->x, this->y);
        }

        Coordinate(const Coordinate&) = default;
        Coordinate(Coordinate&&) noexcept = default;

        template<typename OtherSRef>
        Coordinate(const Coordinate<OtherSRef>& other)
        {
            std::tie(x, y, z) = std::tie(other.x, other.y, other.z);
            sref::detail::convert<OtherSRef, SRef>(x, y, z);
        }

        Coordinate& operator=(const Coordinate&) = default;
        Coordinate& operator=(Coordinate&&) noexcept = default;

        template<typename OtherSRef>
        Coordinate& operator=(const Coordinate<OtherSRef>& other)
        {
            std::tie(x, y, z) = std::tie(other.x, other.y, other.z);
            sref::detail::convert<OtherSRef, SRef>(x, y, z);

            return *this;
        }

        template<typename OtherSRef>
        Coordinate<OtherSRef> as() const
        {
            return Coordinate<OtherSRef>{ *this };
        }

        double2 point() const
        {
            return double2{ x, y };
        }

        double3 point3D() const
        {
            return double3{ x, y, z };
        }

        double3 projectivePoint() const
        {
            return double3{ x, y, 1.0 };
        }

        operator bool() const
        {
            return !isnan();
        }

        Coordinate withX(double otherX) const
        {
            return { otherX, y, z };
        }

        Coordinate withY(double otherY) const
        {
            return { x, otherY, z };
        }

        Coordinate withZ(double otherZ) const
        {
            return { x, y, otherZ };
        }

        Coordinate operator-(const Coordinate& other) const
        {
            return Coordinate(x - other.x, y - other.y, z - other.z);
        }

        Coordinate operator+(const Coordinate& other) const
        {
            return Coordinate(x + other.x, y + other.y, z + other.z);
        }

        Coordinate operator*(double scaleFactor) const
        {
            return Coordinate(x * scaleFactor, y * scaleFactor, z * scaleFactor);
        }

        Coordinate operator/(double scaleFactor) const
        {
            return *this * (1 / scaleFactor);
        }
    };

    using Coordinate3857 = Coordinate<sref::EPSG3857>;
    using Coordinate3857S = Coordinate<sref::ScaledEPSG3857>;
    using Coordinate4326 = Coordinate<sref::EPSG4326>;
    using CoordinateECEF = Coordinate<sref::ECEF>;
    using CoordinateECEFS = Coordinate<sref::ScaledECEF>;
}
