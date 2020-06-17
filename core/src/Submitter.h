
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

#ifndef _SUBMITTER_H
#define _SUBMITTER_H

#include <memory>
#include <thread>
#include <vector>

#include "json11.hpp"

#include "config_json.h"
#include "log.h"
#include "utils.h"

#include "DatasetFrames.h"
#include "RelevanceScores.h"

#include "KeywordRanker.h"
using ImageKeywords = KeywordRanker;

using namespace json11;

class Submitter
{
	std::vector<std::thread> submit_threads;
	std::vector<std::unique_ptr<bool>> finish_flags;
	std::vector<Json> backlog;

	int64_t last_submit_timestamp;

	const SubmitterConfig cfg;

public:
	Submitter(const SubmitterConfig &config);
	// waits until the last thread submits
	~Submitter() noexcept;

	// checks for terminated threads and logging timeout (call on each
	// frame)
	void poll();

	size_t n_submitters() { return submit_threads.size(); }

	/** Called whenever we want to submit frame/shot into the server */
	void submit_and_log_submit(const DatasetFrames &frames,
	                           DisplayType disp_type,
	                           ImageId frame_ID);

	/** Called whenever we rescore (Bayes/LD) */
	void submit_and_log_rescore(const DatasetFrames &frames,
	                            const ScoreModel &scores,
	                            const UsedTools &used_tools,
	                            DisplayType disp_type,
	                            const std::vector<ImageId> &topn_imgs,
	                            const std::string &sentence_query,
	                            const size_t topn_frames_per_video,
	                            const size_t topn_frames_per_shot);

	void log_add_keywords(const std::string &query_sentence);

	void log_like(const DatasetFrames &frames,
	              DisplayType disp_type,
	              ImageId frame_ID);

	void log_dislike(const DatasetFrames &frames,
	                 DisplayType disp_type,
	                 ImageId frame_ID);

	void log_show_som_display(const DatasetFrames &frames,
	                          const std::vector<ImageId> &imgs);

	void log_show_random_display(const DatasetFrames &frames,
	                             const std::vector<ImageId> &imgs);

	void log_show_topn_display(const DatasetFrames &frames,
	                           const std::vector<ImageId> &imgs);

	void log_show_topn_context_display(const DatasetFrames &frames,
	                                   const std::vector<ImageId> &imgs);

	void log_show_topknn_display(const DatasetFrames &frames,
	                             ImageId frame_ID,
	                             const std::vector<ImageId> &imgs);

	void log_show_detail_display(const DatasetFrames &frames,
	                             ImageId frame_ID);

	void log_show_video_replay(const DatasetFrames &frames,
	                           ImageId frame_ID);

	void log_scroll(const DatasetFrames &frames,
	                DisplayType from_disp_type,
	                float dirY);

	void log_reset_search();

private:
	void start_sender(const std::string &submit_url,
	                  const std::string &query_string,
	                  const std::string &post_data);

	void send_query_with_backlog(const std::string &query_string);

	/** @mk We won't be callling this explicitly from the outside, will we?
	 */
	void send_backlog_only();

	/** Called by @ref submit_and_log_submit */
	void log_submit(const DatasetFrames &frames,
	                DisplayType disp_type,
	                ImageId frame_ID);

	/** Called by @ref submit_and_log_rescore */
	void log_rerank(const DatasetFrames &frames,
	                DisplayType from_disp_type,
	                const std::vector<ImageId> &topn_imgs);

	void push_event(const std::string &cat,
	                const std::string &type,
	                const std::string &value);
};

#endif
