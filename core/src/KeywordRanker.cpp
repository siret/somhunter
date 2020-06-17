
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

#include "KeywordRanker.h"

#include <cmath>

std::vector<Keyword>
KeywordRanker::parse_kw_classes_text_file(const std::string &filepath)
{
	std::ifstream inFile(filepath.c_str(), std::ios::in);

	info("loading keyword classes from " << filepath);

	if (!inFile) {
		throw std::runtime_error("Error opening file: " + filepath);
	}

	std::vector<Keyword> result_keywords;

	// read the input file by lines
	for (std::string line_text_buffer;
	     std::getline(inFile, line_text_buffer);) {

		std::stringstream line_buffer_ss(line_text_buffer);

		std::vector<std::string> tokens;

		// Tokenize this line
		for (std::string token;
		     std::getline(line_buffer_ss, token, ':');) {
			tokens.push_back(token);
		}

		// Parse wordnet synset ID
		SynsetId synset_ID{ str2<SynsetId>(tokens[1]) };
		ImageId vec_idx{ ImageId(synset_ID) };

		// String representations
		SynsetStrings synset_strings{ tokens[0] };

		// Top exmaple images
		std::vector<ImageId> top_ex_imgs;
#if TOP_IMAGES_IN_FILE
		for (std::stringstream top_ex_imgs_ss(tokens[2]);
		     std::getline(top_ex_imgs_ss, token, '#');) {
			top_ex_imgs.push_back(str2<int>(token));
		}
#endif

		std::string description;
#if IS_DESCRIPTION_IN_FILE
		//\todo Add descr
#endif

		Keyword k{ vec_idx,
			   synset_ID,
			   std::move(synset_strings),
			   std::move(description),
			   std::move(top_ex_imgs) };

		// Insert this keyword
		result_keywords.emplace_back(std::move(k));
	}

	// Sort them by their ID
	std::sort(
	  result_keywords.begin(),
	  result_keywords.end(),
	  [](const Keyword &l, const Keyword &r) { return l.kw_ID < r.kw_ID; });
	info("keyword classes loaded");

	return result_keywords;
}

FeatureVector
KeywordRanker::parse_float_vector(const std::string &filepath,
                                  size_t dim,
                                  size_t begin_offset)
{
	// Open file for reading as binary from the end side
	std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);

	// If failed to open file
	if (!ifs)
		throw std::runtime_error("Error opening file: " + filepath);

	// Get end of file
	auto end = ifs.tellg();

	// Get iterator to begining
	ifs.seekg(0, std::ios::beg);

	// Compute size of file
	auto size = std::size_t(end - ifs.tellg());

	// If emtpy file
	if (size == 0) {
		throw std::runtime_error("Empty file opened!");
	}

	// Calculate byte length of each row (dim_N * sizeof(float))
	size_t row_byte_len = dim * sizeof(float);

	// Create line buffer
	std::vector<char> line_byte_buffer;
	line_byte_buffer.resize(row_byte_len);

	// Start reading at this offset
	ifs.ignore(begin_offset);

	// Initialize vector of floats for this row
	std::vector<float> features_vector;
	features_vector.reserve(dim);

	// Read binary "lines" until EOF
	while (ifs.read(line_byte_buffer.data(), row_byte_len)) {
		size_t curr_offset = 0;

		// Iterate through all floats in a row
		for (size_t i = 0; i < dim; ++i) {
			features_vector.emplace_back(*reinterpret_cast<float *>(
			  line_byte_buffer.data() + curr_offset));

			curr_offset += sizeof(float);
		}

		// Read just one line
		break;
	}

	return features_vector;
}

FeatureMatrix
KeywordRanker::parse_float_matrix(const std::string &filepath,
                                  size_t row_dim,
                                  size_t begin_offset)
{
	// Open file for reading as binary from the end side
	std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);

	// If failed to open file
	if (!ifs) {
		throw std::runtime_error("Error opening file: " + filepath);
	}

	// Get end of file
	auto end = ifs.tellg();

	// Get iterator to begining
	ifs.seekg(0, std::ios::beg);

	// Compute size of file
	auto size = std::size_t(end - ifs.tellg());

	// If emtpy file
	if (size == 0) {
		throw std::runtime_error("Empty file opened!");
	}

	// Calculate byte length of each row (dim_N * sizeof(float))
	size_t row_byte_len = row_dim * sizeof(float);

	// Create line buffer
	std::vector<char> line_byte_buffer;
	line_byte_buffer.resize(row_byte_len);

	// Start reading at this offset
	ifs.ignore(begin_offset);

	// Declare result structure
	std::vector<std::vector<float>> result_features;

	// Read binary "lines" until EOF
	while (ifs.read(line_byte_buffer.data(), row_byte_len)) {
		// Initialize vector of floats for this row
		std::vector<float> features_vector;
		features_vector.reserve(row_dim);

		size_t curr_offset = 0;

		// Iterate through all floats in a row
		for (size_t i = 0; i < row_dim; ++i) {
			features_vector.emplace_back(*reinterpret_cast<float *>(
			  line_byte_buffer.data() + curr_offset));

			curr_offset += sizeof(float);
		}

		// Insert this row into the result
		result_features.emplace_back(std::move(features_vector));
	}

	return result_features;
}

KwSearchIds
KeywordRanker::find(const std::string &search, size_t num_limit) const
{
	KwSearchIds r;
	KwSearchIds r2;

	for (KeywordId i = 0; i < keywords.size(); ++i)
		for (size_t j = 0; j < keywords[i].synset_strs.size(); ++j) {

			auto kw = keywords[i];

			auto &s = kw.synset_strs[j];
			auto f = s.find(search);

			if (f == std::basic_string<char>::npos)
				continue;

			if (f == 0u)
				r.emplace_back(kw.kw_ID, j);
			else
				r2.emplace_back(kw.kw_ID, j);
		}

	std::sort(
	  r.begin(), r.end(), [&](const KwSearchId &a, const KwSearchId &b) {
		  return keywords[a.first].synset_strs[a.second] <
		         keywords[b.first].synset_strs[b.second];
	  });

	r.insert(r.end(), r2.begin(), r2.end());

	KwSearchIds res;
	for (size_t i = 0; i < num_limit; ++i) {
		if (i >= r.size())
			break;

		res.emplace_back(r[i]);
	}

	return res;
}

void
KeywordRanker::rank_sentence_query(const std::string &sentence_query_raw,
                                   ScoreModel &model,
                                   const DatasetFeatures &features,
                                   const DatasetFrames &frames,
                                   const Config &cfg) const
{
	// Copy this sentence
	std::string sentence_query(sentence_query_raw);

	// Remove all unwanted charactes
	std::string illegal_chars = "\\/?!,.'\"";
	std::transform(sentence_query.begin(),
	               sentence_query.end(),
	               sentence_query.begin(),
	               [&illegal_chars](char c) {
		               // If found in illegal, make it space
		               if (illegal_chars.find(c) != std::string::npos)
			               return ' ';

		               return c;
	               });

	std::stringstream query_ss(sentence_query);

	std::string token_str;
	std::vector<std::string> query;
	while (query_ss >> token_str) {
		query.emplace_back(token_str);
	}

	//
	// Rank it
	//
	if (query.empty())
		return;

	std::vector<std::vector<KeywordId>> pos;
	std::vector<std::vector<KeywordId>> neg;

	std::vector<KeywordId> pos_one_query;
	// Split tokens into temporal queries
	for (const auto &kw_word : query) {

		// If temp query separator
		if (kw_word == ">>" || kw_word == ">") {
			if (pos_one_query.empty())
				continue;

			// This query is done, deploy it
			pos.emplace_back(std::move(pos_one_query));
			pos_one_query.clear();
			continue;
		}

		auto v = find(kw_word);

		if (!v.empty())
			pos_one_query.emplace_back(v.front().first);
	}

	// Deploy this last query
	if (!pos_one_query.empty())
		pos.emplace_back(std::move(pos_one_query));

	rank_query(pos, neg, model, features, frames, cfg);
}

void
KeywordRanker::rank_query(const std::vector<std::vector<KeywordId>> &positive,
                          const std::vector<std::vector<KeywordId>> &negative,
                          ScoreModel &model,
                          const DatasetFeatures &features,
                          const DatasetFrames &frames,
                          const Config &cfg) const
{
	// Don't waste time
	if (positive.empty())
		return;

	// Get the most relevant images for this query
	//  Distance is from [0, 1]
	std::vector<std::pair<ImageId, float>> sorted_frames =
	  get_sorted_frames(positive, negative, features, frames, cfg);

	// Update the model
	for (auto &&[frame_ID, dist] : sorted_frames) {
		model.adjust(frame_ID, std::exp(dist * -42));
	}

	model.normalize();
}

void
KeywordRanker::apply_temp_queries(std::vector<std::vector<float>> &dist_cache,
                                  ImageId img_ID,
                                  const FeatureMatrix &queries,
                                  size_t query_idx,
                                  float &result_dist,
                                  const DatasetFeatures &features,
                                  const DatasetFrames &frames) const
{
	// If no queries left
	if (query_idx >= queries.size())
		return;

	// To avoid getting stuck in loooooong computation
	if (query_idx > MAX_NUM_TEMP_QUERIES)
		return;

	float local_min_dist = 1.0f;

	auto img_it = frames.get_frame_it(img_ID);
	VideoId vid_ID = img_it->video_ID;

	// Iterate over successor frames
	for (size_t i_succ = 0; i_succ < KW_TEMPORAL_SPAN; ++i_succ) {
		++img_it;
		if (img_it == frames.end() || img_it->video_ID != vid_ID)
			break;

		// Compute self distance
		// Get cosine distance and scale it to [0.0f, 1.0f]
		float dist_i_succ = dist_cache[query_idx][img_it->frame_ID];
		// If not yet cached
		if (std::isnan(dist_i_succ)) {
			dist_i_succ =
			  d_cos_normalized(queries[query_idx],
			                   features.fv(img_it->frame_ID),
			                   queries[query_idx].size()) /
			  2.0f;
			dist_cache[query_idx][img_it->frame_ID] = dist_i_succ;
		}

		// Recurse on next queries, this call wil adjust `dist_i_succ`
		apply_temp_queries(dist_cache,
		                   img_it->frame_ID,
		                   queries,
		                   query_idx + 1,
		                   dist_i_succ,
		                   features,
		                   frames);

		// Update minimum
		local_min_dist = std::min(local_min_dist, dist_i_succ);
	}

	// Write new dist to the parameter value
	// @todo what operation do we do here???
	result_dist = result_dist * local_min_dist;
}

std::vector<std::pair<ImageId, float>>
KeywordRanker::get_sorted_frames(
  const std::vector<std::vector<KeywordId>> &positive,
  const std::vector<std::vector<KeywordId>> & /*negative*/,
  const DatasetFeatures &features,
  const DatasetFrames &frames,
  const Config &cfg) const
{
	const size_t result_dim = cfg.kw_PCA_mat_dim;

	std::vector<std::vector<float>> query_vecs;

	for (auto &&kw_IDs : positive) {
		//
		// Embed keywords
		//
		// Initialize zero vector
		std::vector<float> score_vec(kw_pca_mean_vec.size(), 0.0f);

		// Accumuate scores for given keywords
		for (auto &&ID : kw_IDs) {
			score_vec = VecAdd(score_vec, kw_features[ID]);
		}

		// Add bias
		score_vec = VecAdd(score_vec, kw_features_bias_vec);

		// Apply hyperbolic tangent function
		std::transform(
		  score_vec.begin(),
		  score_vec.end(),
		  score_vec.begin(),
		  [](const float &score) { return std::tanh(score); });

		std::vector<float> sentence_vec =

		  MatVecProd(kw_pca_mat,
		             VecSub(VecNorm(score_vec), kw_pca_mean_vec));

		sentence_vec = VecNorm(sentence_vec);

		query_vecs.emplace_back(std::move(sentence_vec));
	}

	std::vector<std::vector<float>> dist_cache;
	dist_cache.resize(query_vecs.size(),
	                  std::vector(features.size(),
	                              std::numeric_limits<float>::quiet_NaN()));

	std::vector<std::pair<ImageId, float>> scores;
	for (size_t img_ID = 0; img_ID < features.size(); ++img_ID) {
		const float *p_img_fea_vec = features.fv(img_ID);

		// Get cosine distance and scale it to [0.0f, 1.0f]
		float dist = dist_cache[0][img_ID];
		// If not yet cached
		if (std::isnan(dist)) {
			dist = d_cos_normalized(query_vecs.front(),
			                        p_img_fea_vec,
			                        result_dim) /
			       2.0f;
			dist_cache[0][img_ID] = dist;
		} else {
			std::cout << "cached" << std::endl;
			;
		}

		// This will adjust `dist` based on temporal queries
		apply_temp_queries(
		  dist_cache, img_ID, query_vecs, 1, dist, features, frames);

		scores.emplace_back(ImageId(img_ID), dist);
	}

	// Sort results
	std::sort(scores.begin(),
	          scores.end(),
	          [](const std::pair<size_t, float> &left,
	             const std::pair<size_t, float> &right) {
		          return left.second < right.second;
	          });

	return scores;
}
