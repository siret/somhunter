
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

#ifndef IMAGE_KEYWORDS_W2VV_H_
#define IMAGE_KEYWORDS_W2VV_H_

#include <cassert>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "DatasetFrames.h"
#include "RelevanceScores.h"
#include "common.h"
#include "config_json.h"
#include "utils.h"

struct Keyword
{
	KeywordId kw_ID;
	SynsetId synset_ID;
	SynsetStrings synset_strs;
	KwDescription desc;

	/** Best representative images for this keyword */
	std::vector<ImageId> top_ex_imgs;
};

class KeywordRanker
{
	std::vector<Keyword> keywords;
	FeatureMatrix kw_features;
	FeatureVector kw_features_bias_vec;
	FeatureMatrix kw_pca_mat;
	FeatureVector kw_pca_mean_vec;

public:
	static std::vector<Keyword> parse_kw_classes_text_file(
	  const std::string &filepath);

	/**
	 * Parses float matrix from a binary file that is written in row-major
	 * format and starts at `begin_offset` offset.k FORMAT: Matrix of 4B
	 * floats, row - major:
	 *    - each line is dim_N * 4B floats
	 *    - number of lines is number of selected frames
	 */
	// @todo Make this template and inside some `Parsers` class
	static FeatureMatrix parse_float_matrix(const std::string &filepath,
	                                        size_t row_dim,
	                                        size_t begin_offset = 0);
	/**
	 * FORMAT:
	 *    Matrix of 4B floats:
	 *    - each line is dim * 4B floats
	 *    - number of lines is number of selected frames
	 */
	// @todo Make this template and inside some `Parsers` class
	static FeatureVector parse_float_vector(const std::string &filepath,
	                                        size_t dim,
	                                        size_t begin_offset = 0);

	inline KeywordRanker(const Config &config)
	  : keywords(parse_kw_classes_text_file(config.kws_file))
	  , kw_features(parse_float_matrix(config.kw_scores_mat_file,
	                                   config.pre_PCA_features_dim))
	  , kw_features_bias_vec(
	      parse_float_vector(config.kw_bias_vec_file,
	                         config.pre_PCA_features_dim))
	  , kw_pca_mat(parse_float_matrix(config.kw_PCA_mat_file,
	                                  config.pre_PCA_features_dim))
	  , kw_pca_mean_vec(parse_float_vector(config.kw_bias_vec_file,
	                                       config.pre_PCA_features_dim))
	{}

	KeywordRanker(const KeywordRanker &) = delete;
	KeywordRanker &operator=(const KeywordRanker &) = delete;

	KeywordRanker(KeywordRanker &&) = default;
	KeywordRanker &operator=(KeywordRanker &&) = default;
	~KeywordRanker() noexcept = default;

	/**
	 * Gets all string representants of this keyword.
	 */
	const Keyword &operator[](KeywordId idx) const
	{
		// Get all keywords with this Keyword ID
		return keywords[idx];
	}

	KwSearchIds find(const std::string &search,
	                 size_t num_limit = 10) const;

	void rank_query(const std::vector<std::vector<KeywordId>> &positive,
	                const std::vector<std::vector<KeywordId>> &negative,
	                ScoreModel &model,
	                const DatasetFeatures &features,
	                const DatasetFrames &frames,
	                const Config &cfg) const;

	void rank_sentence_query(const std::string &sentence_query_raw,
	                         ScoreModel &model,
	                         const DatasetFeatures &features,
	                         const DatasetFrames &frames,
	                         const Config &cfg) const;

private:
	void apply_temp_queries(std::vector<std::vector<float>> &dist_cache,
	                        ImageId img_ID,
	                        const FeatureMatrix &queries,
	                        size_t query_idx,
	                        float &result_dist,
	                        const DatasetFeatures &features,
	                        const DatasetFrames &frames) const;

	/**
	 * Sorts all images based on provided query and retrieves vector
	 * of image IDs with their distance from the query.
	 *
	 */
	std::vector<std::pair<ImageId, float>> get_sorted_frames(
	  const std::vector<std::vector<KeywordId>> &positive,
	  const std::vector<std::vector<KeywordId>> &negative,
	  const DatasetFeatures &features,
	  const DatasetFrames &frames,
	  const Config &cfg) const;
};

#endif // IMAGE_KEYWORDS_W2VV_H_
