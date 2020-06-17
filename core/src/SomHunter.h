
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

#ifndef somhunter_h
#define somhunter_h

#include <set>
#include <string>
#include <vector>

#include "AsyncSom.h"
#include "DatasetFeatures.h"
#include "DatasetFrames.h"
#include "KeywordRanker.h"
#include "RelevanceScores.h"
#include "Submitter.h"

/* This is the main backend class. */

class SomHunter
{
	// *** LOADED DATASET ***
	DatasetFrames frames;
	const DatasetFeatures features;
	const KeywordRanker keywords;
	const Config config;

	// *** SEARCH CONTEXT ***
	// Relevance scores
	ScoreModel scores;

	// Used keyword query
	std::string last_text_query;

	// Relevance feedback context
	std::set<ImageId> likes;
	std::set<ImageId> shown_images;

	// Current display context
	std::vector<VideoFramePointer> current_display;
	DisplayType current_display_type{ DisplayType::DNull };

	// asynchronous SOM worker
	AsyncSom asyncSom;

	// VBS logging
	Submitter submitter;
	UsedTools used_tools;

public:
	SomHunter() = delete;
	/** The main ctor with the filepath to the JSON config file */
	inline SomHunter(const Config &cfg)
	  : config(cfg)
	  , frames(cfg)
	  , features(frames, cfg)
	  , scores(frames)
	  , keywords(cfg)
	  , asyncSom(cfg)
	  , submitter(cfg.submitter_config)
	{
		asyncSom.start_work(features, scores);
	}

	/** Returns display of desired type
	 *
	 *	Some diplays may even support paging (e.g. top_n) or
	 * selection of one frame (e.g. top_knn)
	 */
	FramePointerRange get_display(DisplayType d_type,
	                              ImageId selected_image = 0,
	                              PageId page = 0);

	void add_likes(const std::vector<ImageId> &likes);

	void remove_likes(const std::vector<ImageId> &likes);

	std::vector<const Keyword *> autocomplete_keywords(
	  const std::string &prefix,
	  size_t count) const;

	/**
	 * Applies all algorithms for score
	 * computation and updates context.
	 */
	void rescore(const std::string &text_query);

	bool som_ready() const;

	/** Sumbits frame with given id to VBS server */
	void submit_to_server(ImageId frame_id);

	/** Resets current search context and starts new search */
	void reset_search_session();

private:
	/**
	 *	Applies text query from the user.
	 */
	void rescore_keywords(const std::string &query);

	/**
	 *	Applies feedback from the user based
	 * on shown_images.
	 */
	void rescore_feedback();

	/**
	 *	Gives SOM worker new work.
	 */
	void som_start();

	FramePointerRange get_random_display();

	FramePointerRange get_topn_display(PageId page);

	FramePointerRange get_topn_context_display(PageId page);

	FramePointerRange get_som_display();

	FramePointerRange get_video_detail_display(ImageId selected_image);

	FramePointerRange get_topKNN_display(ImageId selected_image,
	                                     PageId page);

	// Gets only part of last display
	FramePointerRange get_page_from_last(PageId page);

	void reset_scores();
};

#endif
