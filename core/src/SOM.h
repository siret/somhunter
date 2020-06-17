
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

#ifndef embedsom_h
#define embedsom_h

#include <random>
#include <vector>

void
som(size_t n,
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
    std::mt19937 &rng);

void
mapPointsToKohos(size_t n,
                 size_t k,
                 size_t dim,
                 const std::vector<float> &points,
                 const std::vector<float> &koho,
                 std::vector<size_t> &mapping);
#endif
