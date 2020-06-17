
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

#ifndef scores_h
#define scores_h

#include <map>
#include <set>
#include <vector>

#include "DatasetFeatures.h"
#include "DatasetFrames.h"

class ScoreModel
{
	// assert: all scores are always > 0
	std::vector<float> scores;

public:
	ScoreModel(const DatasetFrames &p)
	  : scores(p.size(), 1.0f)
	{}

	void reset()
	{
		for (auto &i : scores)
			i = 1.0f;
	}

	// hard remove image
	float adjust(ImageId i, float prob) { return scores[i] *= prob; }
	float set(ImageId i, float prob) { return scores[i] = prob; }
	float operator[](ImageId i) const { return scores[i]; }
	const float *v() const { return scores.data(); }
	size_t size() const { return scores.size(); }
	void normalize();

	/**
	 * Applies relevance feedback based on
	 * bayesian update rule.
	 */
	void apply_bayes(std::set<ImageId> likes,
	                 const std::set<ImageId> &screen,
	                 const DatasetFeatures &features);

	// gets images with top scores and skips first offset
	std::vector<ImageId> top_n(const DatasetFrames &frames,
	                           size_t n,
	                           size_t from_vid_limit = 0,
	                           size_t from_shot_limit = 0) const;
	// gets images with top scores with temporal context
	std::vector<ImageId> top_n_with_context(const DatasetFrames &frames,
	                                        size_t n,
	                                        size_t from_vid_limit,
	                                        size_t from_shot_limit) const;
	std::vector<ImageId> weighted_sample(size_t k, float pow = 1) const;
	ImageId weighted_example(const std::vector<ImageId> &subset) const;
	size_t rank_of_image(ImageId i) const;
};

#endif
