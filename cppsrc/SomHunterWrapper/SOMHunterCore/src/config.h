
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

#ifndef config_h
#define config_h

#include "common.h"

/*
 * Text query settings
 */

#define MAX_NUM_TEMP_QUERIES 2
#define KW_TEMPORAL_SPAN 5 // frames

/*
 * Scoring/SOM stuff
 */
/*
#define TOPN_PER_VIDEO_NUM_FRAMES_LIMIT 3
#define TOPKNN_PER_VIDEO_NUM_FRAMES_LIMIT 3
#define PER_VIDEO_NUM_FROM_SHOT_LIMIT 1
*/
#define TOPN_LIMIT 10000
constexpr size_t DISP_TOPN_CTX_RESULT_LIMIT = 10000;
#define TOPKNN_LIMIT 10000
#define SOM_ITERS 100000

/*
 * Misc
 */

/** What display we'll jump to after a rescore. */
constexpr DisplayType POST_RESCORE_DISPLAY = DisplayType::DTopNContext;

#define DEFAULT_RESCORE 0 // 0=bayes, 1=LD

#define FADE_ON_HOVER 0
#define DRAW_FRAME_INFO 0
#define DRAW_FRAME_SCORES 0

#define ALLOW_DEBUG_WINDOW 1
/*
 * Logging
 */

#define LOGLEVEL 1 // 3 = debug, 2 = info, 1 = warnings, 0 = none

#define LD_LOG_DIR "./logs/"
#define LD_LOG_FILENAME "ld"
#define FIRST_SHOWN_LOG_FILENAME "first_shown"

#define GLOBAL_LOG_FILE "somhunter.log"

/** Pop-up window image grid width */
#define DISPLAY_GRID_WIDTH 6

/** Pop-up window image grid height */
#define DISPLAY_GRID_HEIGHT 6

constexpr int TOP_N_SELECTED_FRAME_POSITION = 2;
constexpr float RANDOM_DISPLAY_WEIGHT = 3.0f;

/** SOM window image grid width */
#define SOM_DISPLAY_GRID_WIDTH 8

/** SOM window image grid height */
#define SOM_DISPLAY_GRID_HEIGHT 8

#endif
