
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
#ifndef use_intrins_h

/* MSVC does not define SSE* flags therefore we just try to turn in on,
 * if your CPU does not support this, you'll get compilation error. */
#if (defined(__SSE4_2__) || defined(_MSC_VER))
#define USE_INTRINS
#else
#pragma message("Fix your CXXFLAGS or get a better CPU!")
#endif

#ifdef USE_INTRINS
#if defined(_MSC_VER)
#include <wmmintrin.h>
#else
#include <xmmintrin.h>
#endif
#endif

#endif // use_intrins_h
