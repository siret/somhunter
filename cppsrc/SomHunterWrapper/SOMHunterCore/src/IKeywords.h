
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

#ifndef I_KEYWORDS_H_
#define I_KEYWORDS_H_

#include <cassert>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Frames.h"
#include "Scores.h"
#include "config.h"

// d a t a
struct Keyword
{
	KeywordId kw_ID;
	SynsetId synset_ID;
	SynsetStrings synset_strs;
	KwDescription desc;

	/** Best representative images for this keyword */
	std::vector<ImageId> top_ex_imgs;
};

#endif // I_KEYWORDS_H_
