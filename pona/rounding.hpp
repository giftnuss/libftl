/*
 * rounding.hpp -- rounding floating points numbers to zero or infinite
 *
 * Copyright (c) 2007-2010, Frank Mertens
 *
 * See ../LICENSE for the license.
 */
#ifndef PONA_ROUNDING_HPP
#define PONA_ROUNDING_HPP

#include <math.h>

namespace pona
{

inline double roundToZero(double x)
{
	double ip; // integral part
	double fp = modf(x, &ip); // fractional part;
	return ip - int(fp < -0.5) + int(0.5 < fp);
}

inline double ceilToInf(double x)
{
	double ip; // integral part
	double fp = modf(x, &ip); // fractional part;
	return ip - int(fp < 0) + int(0 < fp);
}

inline double roundToInf(double x)
{
	double ip; // integral part
	double fp = modf(x, &ip); // fractional part;
	return ip - int(fp <= -0.5) + int(0.5 <= fp);
}


inline double floorToZero(double x)
{
	double ip; // integral part
	modf(x, &ip);
	return ip;
}

inline double floorToMinusInf(double x)
{
	double ip; // integral part
	double fp = modf(x, &ip); // fractional part;
	return ip - int(fp < 0);
}

template<class T>
inline int sign(T x) { return 1 - 2 * (x < 0); }

template<class T>
inline int sgn(T x) { return (0 < x) - (x < 0); }

template<class T>
inline T bound(T min, T x, T max) {
	if (x < min) x = min;
	else if (x > max) x = max;
	return x;
}

} // namespace pona

#endif // PONA_ROUNDING_HPP