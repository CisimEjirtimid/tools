#pragma once
#include "static_vec.h"

namespace mdsp::spatial
{
    static constexpr double WGS84_A = +6.37813700000000000000e+0006;        /* a */
    static constexpr double WGS84_INVF = +2.98257223563000000000e+0002;     /* 1/f */
    static constexpr double WGS84_F = +3.35281066474748071998e-0003;        /* f */
    static constexpr double WGS84_INVA = +1.56785594288739799723e-0007;     /* 1/a */
    static constexpr double WGS84_B = +6.35675231424517949745e+0006;        /* b */
    static constexpr double WGS84_C = +5.21854008423385332406e+0005;        /* c */
    static constexpr double WGS84_E = +8.18191908426214947083e-0002;        /* e */
    static constexpr double WGS84_EE = +6.69437999014131705734e-0003;       /* e^2 */
    static constexpr double WGS84_P1MEEDB = +1.56259921876129741211e-0007;  /* (1-(e^2))/b */

    static constexpr double WGS84_INVAA = +2.45817225764733181057e-0014;    /* 1/(a^2) */
    static constexpr double WGS84_AADC = +7.79540464078689228919e+0007;     /* (a^2)/c */
    static constexpr double WGS84_BBDCC = +1.48379031586596594555e+0002;    /* (b^2)/(c^2) */
    static constexpr double WGS84_EED2 = +3.34718999507065852867e-0003;     /* (e^2)/2 */
    static constexpr double WGS84_P1MEE = +9.93305620009858682943e-0001;    /* 1-(e^2) */
    static constexpr double WGS84_P1MEEDAA = +2.44171631847341700642e-0014; /* (1-(e^2))/(a^2) */
    static constexpr double WGS84_HMIN = +2.25010182030430273673e-0014;     /* (e^12)/4 */
    static constexpr double WGS84_EEEE = +4.48147234524044602618e-0005;     /* e^4 */
    static constexpr double WGS84_EEEED4 = +1.12036808631011150655e-0005;   /* (e^4)/4 */
    static constexpr double WGS84_INVCBRT2 = +7.93700525984099737380e-0001; /* 1/(2^(1/3)) */
    static constexpr double WGS84_INV3 = +3.33333333333333333333e-0001;     /* 1/3 */
    static constexpr double WGS84_INV6 = +1.66666666666666666667e-0001;     /* 1/6 */
    static constexpr double WGS84_D2R = +1.74532925199432957691e-0002;      /* pi/180 */
    static constexpr double WGS84_R2D = +5.72957795130823208766e+0001;      /* 180/pi */

    // geodetic (latitude, longitude, altitude)
    // ecef (x, y, z)
    // corresponding ellipsoid WGS84
    // method from https://hal.science/hal-01704943/file/AccurateEcefConversion-31oct2019.pdf
    inline double3 convertGeodeticToECEF(const double3& geodetic)
    {
        double lat = math::deg2rad(geodetic[0]);
        double lon = math::deg2rad(geodetic[1]);
        double alt = geodetic[2];
        double coslat = std::cos(lat);
        double sinlat = std::sin(lat);
        double coslon = std::cos(lon);
        double sinlon = std::sin(lon);
        double N = WGS84_AADC / std::sqrt(coslat * coslat + WGS84_BBDCC);
        double d = (N + alt) * coslat;
        return double3{ d * coslon, d * sinlon, (WGS84_P1MEE * N + alt) * sinlat };
    }

    inline double3 convertECEFToGeodetic(const double3& ecef)
    {
        double lat, lon, alt;

        double ww = ecef[0] * ecef[0] + ecef[1] * ecef[1];
        double m = ww * WGS84_INVAA;
        double n = ecef[2] * ecef[2] * WGS84_P1MEEDAA;
        double mpn = m + n;
        double p = WGS84_INV6 * (mpn - WGS84_EEEE);
        double G = m * n * WGS84_EEEED4;
        double H = 2 * p * p * p + G;

        double C = std::pow(H + G + 2 * std::sqrt(H * G), WGS84_INV3) * WGS84_INVCBRT2;
        double i = -WGS84_EEEED4 - 0.5 * mpn;
        double P = p * p;
        double beta = WGS84_INV3 * i - C - P / C;
        double k = WGS84_EEEED4 * (WGS84_EEEED4 - mpn);

        double t1 = beta * beta - k;
        double t2 = std::sqrt(t1);
        double t3 = t2 - 0.5 * (beta + i);
        double t4 = std::sqrt(t3);

        double t5 = 0.5 * (beta - i);
        t5 = std::fabs(t5);

        double t6 = std::sqrt(t5);
        double t7 = (m < n) ? t6 : -t6;

        double t = t4 + t7;

        double j = WGS84_EED2 * (m - n);
        double g = 2 * j;
        double tttt = t * t * t * t;
        double F = tttt + 2 * i * t * t + g * t + k;
        double dFdt = 4 * t * t * t + 4 * i * t + g;
        double dt = -F / dFdt;

        double u = t + dt + WGS84_EED2;
        double v = t + dt - WGS84_EED2;
        double w = std::sqrt(ww);
        double zu = ecef[2] * u;
        double wv = w * v;
        lat = std::atan2(zu, wv);

        double invuv = 1 / (u * v);
        double dw = w - wv * invuv;
        double dz = ecef[2] - zu * WGS84_P1MEE * invuv;
        double da = std::sqrt(dw * dw + dz * dz);
        alt = (u < 1) ? -da : da;

        lon = std::atan2(ecef[1], ecef[0]);

        return Vec3{ math::rad2deg(lat), math::rad2deg(lon), alt };
    }

    // ned - north, east, down (x, y, z);
    // center of ned - center of camera
    // ecef (x, y, z)
    // center of ecef - center of ellipsoid WGS84
    inline double3 convertNEDToECEF(const double3& ned, const double3& geodeticOrigin)
    {
        double lat = math::deg2rad(geodeticOrigin[0]);
        double lon = math::deg2rad(geodeticOrigin[1]);

        //ned * ned2ecefMatrix
        double3 ecef;

        ecef[0] = -sin(lat) * cos(lon) * ned[0] - sin(lon) * ned[1] - cos(lat) * cos(lon) * ned[2];
        ecef[1] = -sin(lat) * sin(lon) * ned[0] + cos(lon) * ned[1] - cos(lat) * sin(lon) * ned[2];
        ecef[2] = cos(lat) * ned[0] - sin(lat) * ned[2];

        // ecef += ecefRef
        double3 ecefRef;
        ecefRef = convertGeodeticToECEF(geodeticOrigin);

        ecef[0] += ecefRef[0];
        ecef[1] += ecefRef[1];
        ecef[2] += ecefRef[2];

        return ecef;
    }

    double3 convertECEFToNED(const double3& ecef, const double3& geodeticOrigin)
    {
        double3 ecefRef;
        ecefRef = convertGeodeticToECEF(geodeticOrigin);

        // ecef -= ecefRef
        double3 vect;
        vect[0] = ecef[0] - ecefRef[0];
        vect[1] = ecef[1] - ecefRef[1];
        vect[2] = ecef[2] - ecefRef[2];

        double lat = math::deg2rad(geodeticOrigin[0]);
        double lon = math::deg2rad(geodeticOrigin[1]);

        double3 ned;

        //ecef * ecef2nedMatrix
        ned[0] = -sin(lat) * cos(lon) * vect[0] - sin(lat) * sin(lon) * vect[1] + cos(lat) * vect[2];
        ned[1] = -sin(lon) * vect[0] + cos(lon) * vect[1];
        ned[2] = -cos(lat) * cos(lon) * vect[0] - cos(lat) * sin(lon) * vect[1] - sin(lat) * vect[2];

        return ned;
    }
}
