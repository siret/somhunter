
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

#include <stdexcept>

#include "SomHunter.h"

#include "log.h"
#include "utils.h"

FramePointerRange
SomHunter::get_display(DisplayType d_type, ImageId selected_image, PageId page)
{
	submitter.poll();

	switch (d_type) {
		case DisplayType::DRand:
			return get_random_display();

		case DisplayType::DTopN:
			return get_topn_display(page);

		case DisplayType::DTopNContext:
			return get_topn_context_display(page);

		case DisplayType::DSom:
			return get_som_display();

		case DisplayType::DVideoDetail:
			return get_video_detail_display(selected_image);

		case DisplayType::DTopKNN:
			return get_topKNN_display(selected_image, page);

		default:
			warn("Unsupported display requested.");
#ifndef NDEBUG
			throw std::runtime_error(
			  "Unsupported display requested.");
#endif // !NDEBUG

			break;
	}

	return FramePointerRange();
}

void
SomHunter::add_likes(const std::vector<ImageId> &likes)
{
	submitter.poll();

	for (auto ii : likes) {
		this->likes.insert(ii);

		frames.get_frame(ii).liked = true;

		submitter.log_like(frames, current_display_type, ii);
	}
}

void
SomHunter::remove_likes(const std::vector<ImageId> &likes)
{
	submitter.poll();

	for (auto ii : likes) {
		this->likes.erase(ii);

		frames.get_frame(ii).liked = false;

		submitter.log_dislike(frames, current_display_type, ii);
	}
}

std::vector<const Keyword *>
SomHunter::autocomplete_keywords(const std::string &prefix, size_t count) const
{
	// Get the keywrods IDs
	auto kw_IDs{ keywords.find(prefix, count) };

	// Create vector of ptrs to corresponding keyword instances
	std::vector<const Keyword *> res;
	res.reserve(kw_IDs.size());
	for (auto &&kw_ID : kw_IDs) {
		res.emplace_back(&keywords[kw_ID.first]);
	}

	return res;
}

void
SomHunter::rescore(const std::string &text_query)
{
	submitter.poll();

	// Rescore text query
	rescore_keywords(text_query);

	// Rescore relevance feedback
	rescore_feedback();

	// Start SOM computation
	som_start();

	// Update search context
	shown_images.clear();

	// Reset likes
	likes.clear();
	for (auto &fr : frames) {
		fr.liked = false;
	}

	auto top_n = scores.top_n(frames,
	                          TOPN_LIMIT,
	                          config.topn_frames_per_video,
	                          config.topn_frames_per_shot);

	debug("used_tools.topknn_used = " << used_tools.topknn_used);
	debug("used_tools.KWs_used = " << used_tools.KWs_used);
	debug("used_tools.bayes_used = " << used_tools.bayes_used);
	submitter.submit_and_log_rescore(frames,
	                                 scores,
	                                 used_tools,
	                                 current_display_type,
	                                 top_n,
	                                 last_text_query,
	                                 config.topn_frames_per_video,
	                                 config.topn_frames_per_shot);
}

bool
SomHunter::som_ready() const
{
	return asyncSom.map_ready();
}

void
SomHunter::submit_to_server(ImageId frame_id)
{
	submitter.submit_and_log_submit(frames, current_display_type, frame_id);
}

void
SomHunter::reset_search_session()
{
	submitter.poll();

	reset_scores();
	submitter.log_reset_search();
	som_start();
}

void
SomHunter::rescore_keywords(const std::string &query)
{
	// Do not rescore if query did not change
	if (last_text_query == query) {
		return;
	}

	reset_scores();

	keywords.rank_sentence_query(query, scores, features, frames, config);

	last_text_query = query;
	used_tools.KWs_used = true;

	submitter.log_add_keywords(query);
}

void
SomHunter::rescore_feedback()
{
	if (likes.empty())
		return;

	scores.apply_bayes(likes, shown_images, features);
	used_tools.bayes_used = true;
}

void
SomHunter::som_start()
{
	asyncSom.start_work(features, scores);
}

FramePointerRange
SomHunter::get_random_display()
{
	// Get ids
	auto ids = scores.weighted_sample(
	  DISPLAY_GRID_WIDTH * DISPLAY_GRID_HEIGHT, RANDOM_DISPLAY_WEIGHT);

	// Log
	submitter.log_show_random_display(frames, ids);
	// Update context
	for (auto id : ids)
		shown_images.insert(id);
	current_display = frames.ids_to_video_frame(ids);
	current_display_type = DisplayType::DRand;

	return FramePointerRange(current_display);
}

FramePointerRange
SomHunter::get_topn_display(PageId page)
{
	// Another display or first page -> load
	if (current_display_type != DisplayType::DTopN || page == 0) {
		debug("Loading top n display first page");
		// Get ids
		auto ids = scores.top_n(frames,
		                        TOPN_LIMIT,
		                        config.topn_frames_per_video,
		                        config.topn_frames_per_shot);

		// Log
		submitter.log_show_topn_display(frames, ids);

		// Update context
		current_display = frames.ids_to_video_frame(ids);
		current_display_type = DisplayType::DTopN;
	}

	return get_page_from_last(page);
}

FramePointerRange
SomHunter::get_topn_context_display(PageId page)
{
	// Another display or first page -> load
	if (current_display_type != DisplayType::DTopNContext || page == 0) {
		debug("Loading top n context display first page");
		// Get ids
		auto ids =
		  scores.top_n_with_context(frames,
		                            TOPN_LIMIT,
		                            config.topn_frames_per_video,
		                            config.topn_frames_per_shot);

		// Log
		submitter.log_show_topn_context_display(frames, ids);

		// Update context
		current_display = frames.ids_to_video_frame(ids);
		current_display_type = DisplayType::DTopNContext;
	}

	return get_page_from_last(page);
}

FramePointerRange
SomHunter::get_som_display()
{
	if (!asyncSom.map_ready()) {
		return FramePointerRange();
	}

	std::vector<ImageId> ids;
	ids.resize(SOM_DISPLAY_GRID_WIDTH * SOM_DISPLAY_GRID_HEIGHT);

	for (size_t i = 0; i < SOM_DISPLAY_GRID_WIDTH; ++i) {
		for (size_t j = 0; j < SOM_DISPLAY_GRID_HEIGHT; ++j) {
			if (asyncSom.map(i + SOM_DISPLAY_GRID_WIDTH * j)
			      .empty()) {
				ids[i + SOM_DISPLAY_GRID_WIDTH * j] =
				  IMAGE_ID_ERR_VAL;
			} else {
				ImageId id = scores.weighted_example(
				  asyncSom.map(i + SOM_DISPLAY_GRID_WIDTH * j));
				ids[i + SOM_DISPLAY_GRID_WIDTH * j] = id;
			}
		}
	}

	// Log
	submitter.log_show_som_display(frames, ids);

	// Update context
	for (auto id : ids) {
		if (id == IMAGE_ID_ERR_VAL)
			continue;

		shown_images.insert(id);
	}
	current_display = frames.ids_to_video_frame(ids);
	current_display_type = DisplayType::DSom;

	return FramePointerRange(current_display);
}

FramePointerRange
SomHunter::get_video_detail_display(ImageId selected_image)
{
	VideoId v_id = frames.get_video_id(selected_image);

	if (v_id == VIDEO_ID_ERR_VAL) {
		warn("Video for " << selected_image << " not found");
		return std::vector<VideoFramePointer>();
	}

	// Get ids
	FrameRange video_frames = frames.get_all_video_frames(v_id);

	// Log
	submitter.log_show_detail_display(frames, selected_image);

	// Update context
	for (auto iter = video_frames.begin(); iter != video_frames.end();
	     ++iter) {
		shown_images.insert(iter->frame_ID);
	}

	current_display = frames.range_to_video_frame(video_frames);
	current_display_type = DisplayType::DVideoDetail;

	return FramePointerRange(current_display);
}

FramePointerRange
SomHunter::get_topKNN_display(ImageId selected_image, PageId page)
{
	// Another display or first page -> load
	if (current_display_type != DisplayType::DTopKNN || page == 0) {
		debug("Getting KNN for image " << selected_image);
		// Get ids
		auto ids = features.get_top_knn(frames,
		                                selected_image,
		                                config.topn_frames_per_video,
		                                config.topn_frames_per_shot);

		debug("Got result of size " << ids.size());
		// Log
		submitter.log_show_topknn_display(frames, selected_image, ids);

		// Update context
		current_display = frames.ids_to_video_frame(ids);
		current_display_type = DisplayType::DTopKNN;

		debug("Context is ready");

		// KNN is query by example so we NEED to log a rerank
		UsedTools ut;
		ut.topknn_used = true;

		submitter.submit_and_log_rescore(frames,
		                                 scores,
		                                 ut,
		                                 current_display_type,
		                                 ids,
		                                 last_text_query,
		                                 config.topn_frames_per_video,
		                                 config.topn_frames_per_shot);

		debug("Logging is done");
	}

	return get_page_from_last(page);
}

FramePointerRange
SomHunter::get_page_from_last(PageId page)
{
	debug("Getting page "
	      << page << ", page size " << config.display_page_size
	      << ", current display size " << current_display.size());

	size_t begin_off{ std::min(current_display.size(),
		                   page * config.display_page_size) };
	size_t end_off{ std::min(current_display.size(),
		                 page * config.display_page_size +
		                   config.display_page_size) };

	FramePointerRange res(current_display.cbegin() + begin_off,
	                      current_display.cbegin() + end_off);

	// Update context
	for (auto iter = res.begin(); iter != res.end(); ++iter)
		shown_images.insert((*iter)->frame_ID);

	return res;
}

void
SomHunter::reset_scores()
{
	used_tools.reset();

	shown_images.clear();

	// Reset likes
	likes.clear();
	for (auto &fr : frames) {
		fr.liked = false;
	}

	last_text_query = "";

	scores.reset();
}
