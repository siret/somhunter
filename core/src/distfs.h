
/* This file is part of SOMHunter.
 *
 * Copyright (C) 2020 František Mejzlík <frankmejzlik@gmail.com>
 *                    Mirek Kratochvil <exa.exa@gmail.com>
 *                    Patrik Veselý <prtrikvesely@gmail.com>
 *
 * SOMHunter is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or (at your option)
 * any later version.
 *
 * SOMHunter is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * SOMHunter. If not, see <https://www.gnu.org/licenses/>.
 */

// This particular file is relicensed, originating in EmbedSOM software.

#include "use_intrins.h"
#include <algorithm>
#include <cmath>

#ifdef USE_INTRINS

#ifdef _MSC_VER
template<unsigned i>
constexpr inline float
get(__m128 V)
{
	union
	{
		__m128 v;
		float a[4];
	} converter;
	converter.v = V;
	return converter.a[i];
}
#else
template<unsigned i>
constexpr inline float
get(__m128 V)
{
	return V[i];
}
#endif
#endif

static inline float
sqrf(float n)
{
	return n * n;
}

inline static float
d_sqeucl(const float *p1, const float *p2, const size_t dim)
{
#ifndef USE_INTRINS
	float sqdist = 0;
	for (size_t i = 0; i < dim; ++i) {
		float tmp = p1[i] - p2[i];
		sqdist += tmp * tmp;
	}
	return sqdist;
#else
	const float *p1e = p1 + dim, *p1ie = p1e - 3;

	__m128 s = _mm_setzero_ps();
	for (; p1 < p1ie; p1 += 4, p2 += 4) {
		__m128 tmp = _mm_sub_ps(_mm_loadu_ps(p1), _mm_loadu_ps(p2));
		s = _mm_add_ps(_mm_mul_ps(tmp, tmp), s);
	}
	float sqdist = get<0>(s) + get<1>(s) + get<2>(s) + get<3>(s);
	for (; p1 < p1e; ++p1, ++p2) {
		float tmp = *p1 - *p2;
		sqdist += tmp * tmp;
	}
	return sqdist;
#endif
}

#ifdef USE_INTRINS
inline static __m128
abs_mask(void)
{
	__m128i minus1 = _mm_set1_epi32(-1);
	return _mm_castsi128_ps(_mm_srli_epi32(minus1, 1));
}
inline static __m128
vec_abs(__m128 v)
{
	return _mm_and_ps(abs_mask(), v);
}
#endif

inline static float
d_manhattan(const float *p1, const float *p2, const size_t dim)
{
#ifndef USE_INTRINS
	float mdist = 0;
	for (size_t i = 0; i < dim; ++i) {
		mdist += std::abs(p1[i] - p2[i]);
	}
	return mdist;
#else
	const float *p1e = p1 + dim, *p1ie = p1e - 3;

	__m128 s = _mm_setzero_ps();
	for (; p1 < p1ie; p1 += 4, p2 += 4) {
		s = _mm_add_ps(
		  s, vec_abs(_mm_sub_ps(_mm_loadu_ps(p1), _mm_loadu_ps(p2))));
	}
	float mdist = get<0>(s) + get<1>(s) + get<2>(s) + get<3>(s);
	for (; p1 < p1e; ++p1, ++p2) {
		mdist += std::abs(*p1 - *p2);
	}
	return mdist;
#endif
}

inline static float
d_dot(const float *p1, const float *p2, const size_t dim)
{
#ifndef USE_INTRINS
	float mdist = 0;
	for (size_t i = 0; i < dim; ++i) {
		mdist += p1[i] * p2[i];
	}
	return mdist;
#else
	const float *p1e = p1 + dim, *p1ie = p1e - 3;

	__m128 s = _mm_setzero_ps();
	for (; p1 < p1ie; p1 += 4, p2 += 4) {
		s =
		  _mm_add_ps(s, _mm_mul_ps(_mm_loadu_ps(p1), _mm_loadu_ps(p2)));
	}
	float mdist = get<0>(s) + get<1>(s) + get<2>(s) + get<3>(s);
	for (; p1 < p1e; ++p1, ++p2) {
		mdist += *p1 * *p2;
	}
	return mdist;
#endif
}
