
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

#include "SOM.h"

#include <cmath>

#include "distfs.h"
#include "log.h"

// uncomment to get euclidean distances
#define EUCL

#ifdef EUCL
#define DIST_FUNC d_sqeucl
#define UNDIST_FUNC sqrtf
#else
#define DIST_FUNC d_manhattan
#define UNDIST_FUNC
#endif

// this helps with debugging floating-point overflows and similar nastiness,
// uncomment if needed.
//#define DEBUG_CRASH_ON_FPE

#ifdef DEBUG_CRASH_ON_FPE
#include <fenv.h>
#endif

using namespace std;

// some small numbers first!
static const float min_boost = 0.00001f; // lower limit for the parameter

// this is added before normalizing the distances
static const float zero_avoidance = 0.00000001f;

// a tiny epsilon for preventing singularities
static const float koho_gravity = 0.00000001f;

struct dist_id
{
	float dist;
	size_t id;
};

static inline void
hswap(dist_id &a, dist_id &b)
{
	dist_id c = a;
	a = b;
	b = c;
}

static void
heap_down(dist_id *heap, size_t start, size_t lim)
{
	for (;;) {
		size_t L = 2 * start + 1;
		size_t R = L + 1;
		if (R < lim) {
			float dl = heap[L].dist;
			float dr = heap[R].dist;

			if (dl > dr) {
				if (heap[start].dist >= dl)
					break;
				hswap(heap[L], heap[start]);
				start = L;
			} else {
				if (heap[start].dist >= dr)
					break;
				hswap(heap[R], heap[start]);
				start = R;
			}
		} else if (L < lim) {
			if (heap[start].dist < heap[L].dist)
				hswap(heap[L], heap[start]);
			break; // exit safely!
		} else
			break;
	}
}

void
som(size_t /*n*/,
    size_t k,
    size_t dim,
    size_t niter,
    const std::vector<float> &points,
    std::vector<float> &koho,
    const std::vector<float> &nhbrdist,
    const float alphasA[2],
    const float radiiA[2],
    const float alphasB[2],
    const float radiiB[2],
    const std::vector<float> &scores,
    std::mt19937 &rng)
{
	info("build begin");
	std::discrete_distribution<size_t> random(scores.begin(), scores.end());
	info("build end");

	float thresholdA0 = radiiA[0];
	float alphaA0 = alphasA[0];
	float thresholdADiff = radiiA[1] - radiiA[0];
	float alphaADiff = alphasA[1] - alphasA[0];
	float thresholdB0 = radiiB[0];
	float alphaB0 = alphasB[0];
	float thresholdBDiff = radiiB[1] - radiiB[0];
	float alphaBDiff = alphasB[1] - alphasB[0];

	for (size_t iter = 0; iter < niter; ++iter) {
		size_t point = random(rng);
		float riter = iter / float(niter);

		size_t nearest = 0;
		{
			float nearestd = DIST_FUNC(
			  points.data() + dim * point, koho.data(), dim);
			for (size_t i = 1; i < k; ++i) {
				float tmp =
				  DIST_FUNC(points.data() + dim * point,
				            koho.data() + dim * i,
				            dim);
				if (tmp < nearestd) {
					nearest = i;
					nearestd = tmp;
				}
			}
		}

		float thresholdA = thresholdA0 + riter * thresholdADiff;
		float thresholdB = thresholdB0 + riter * thresholdBDiff;
		float alphaA = alphaA0 + riter * alphaADiff;
		float alphaB = alphaB0 + riter * alphaBDiff;

		for (size_t i = 0; i < k; ++i) {
			float d = nhbrdist[i + k * nearest];

			float alpha;

			if (d > thresholdA) {
				if (d > thresholdB)
					continue;
				alpha = alphaB;
			} else
				alpha = alphaA;

			for (size_t j = 0; j < dim; ++j)
				koho[j + i * dim] +=
				  alpha *
				  (points[j + point * dim] - koho[j + i * dim]);
		}
	}
}

/* this serves for classification into small clusters */
void
mapPointsToKohos(size_t n,
                 size_t k,
                 size_t dim,
                 const std::vector<float> &points,
                 const std::vector<float> &koho,
                 std::vector<size_t> &mapping)
{
	for (size_t point = 0; point < n; ++point) {
		size_t nearest = 0;
		float nearestd =
		  DIST_FUNC(points.data() + dim * point, koho.data(), dim);
		for (size_t i = 1; i < k; ++i) {
			float tmp = DIST_FUNC(points.data() + dim * point,
			                      koho.data() + dim * i,
			                      dim);
			if (tmp < nearestd) {
				nearest = i;
				nearestd = tmp;
			}
		}

		mapping[point] = nearest;
	}
}
