
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

#include "Submitter.h"

#include <filesystem>
#include <fstream>
#include <memory>

#include <curl/curl.h>

static void
submitter_thread(const std::string &submit_url,
                 const std::string &query,
                 const std::string &data,
                 bool &finished,
                 const SubmitterConfig &cfg)
{
	/*
	 * write the stuff into a file just to be sure and have it nicely
	 * archived
	 */

	if (!std::filesystem::is_directory(cfg.VBS_submit_archive_dir))
		std::filesystem::create_directory(cfg.VBS_submit_archive_dir);

	if (!std::filesystem::is_directory(cfg.VBS_submit_archive_dir))
		warn("wtf, directory was not created");

	{
		std::string path = cfg.VBS_submit_archive_dir +
		                   std::string("/") +
		                   std::to_string(timestamp()) +
		                   cfg.VBS_submit_archive_log_suffix;
		std::ofstream o(path.c_str(), std::ios::app);
		if (!o) {
			warn("Could not write a log file!");
		} else {
			o << "{"
			  << "\"query_string\": \"" << query << "\","
			  << std::endl
			  << "\"submit_url\": \"" << submit_url << "\""
			  << std::endl;

			// Only print this if not empty
			if (!data.empty())
				o << ","
				  << "\"data\":" << data << std::endl;

			o << "}" << std::endl;
		}
	}

	if (cfg.extra_verbose_log) {

		// Subtract ',' to '\n'
		std::string data_not_so_pretty_fmtd(data);
		std::replace(data_not_so_pretty_fmtd.begin(),
		             data_not_so_pretty_fmtd.end(),
		             ',',
		             '\n');
		std::replace(data_not_so_pretty_fmtd.begin(),
		             data_not_so_pretty_fmtd.end(),
		             '{',
		             '\n');

		std::cout << "**** Query string: " << query << std::endl
		          << std::endl
		          << "**** " << data_not_so_pretty_fmtd << std::endl;
	}

	if (cfg.submit_to_VBS) {
		CURL *curl = curl_easy_init();

		curl_easy_setopt(curl, CURLOPT_HEADER, 0);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);

		std::string url = submit_url;
		url += "?" + query;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());

		// for some reason, curl refuses to work with const char ptrs
		static std::string hdr = "Content-type: application/json";
		static struct curl_slist reqheader = { hdr.data(), nullptr };
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, &reqheader);

		bool curl_ok = curl_easy_perform(curl) == 0u;

		if (curl_ok)
			info("Submit OK");
		else
			warn("Submit failed!");

		curl_easy_cleanup(curl);
	}

	finished = true;
}

Submitter::Submitter(const SubmitterConfig &config)
  : last_submit_timestamp(timestamp())
  , cfg(config)
{}

Submitter::~Submitter()
{
	send_backlog_only();

	for (auto &t : submit_threads)
		t.join();
}

void
Submitter::submit_and_log_submit(const DatasetFrames &frames,
                                 DisplayType disp_type,
                                 ImageId frame_ID)
{
	log_submit(frames, disp_type, frame_ID);

	auto vf = frames.get_frame(frame_ID);

	std::stringstream query_ss;
	query_ss << "team=" << cfg.team_ID << "&member=" << cfg.member_ID
	         << "&video="
	         << (vf.video_ID + 1) //< !! VBS videos start from 1
	         << "&frame="
	         << vf.frame_number; //< !! VBS frame numbers start at 0

	send_query_with_backlog(query_ss.str());
}

void
Submitter::log_submit(const DatasetFrames & /*frames*/,
                      DisplayType /*disp_type*/,
                      ImageId /*frame_ID*/)
{
	// @todo we can log them for our purporses
}

void
Submitter::log_rerank(const DatasetFrames & /*frames*/,
                      DisplayType /*from_disp_type*/,
                      const std::vector<ImageId> & /*topn_imgs*/)
{
	// @todo we can log them for our purporses
}

void
Submitter::send_backlog_only()
{
	send_query_with_backlog("");
}

void
Submitter::submit_and_log_rescore(const DatasetFrames &frames,
                                  const ScoreModel &scores,
                                  const UsedTools &used_tools,
                                  DisplayType /*disp_type*/,
                                  const std::vector<ImageId> &topn_imgs,
                                  const std::string &sentence_query,
                                  const size_t topn_frames_per_video,
                                  const size_t topn_frames_per_shot)
{

	std::vector<Json> results;
	results.reserve(topn_imgs.size());

	for (auto &&img_ID : topn_imgs) {
		auto vf = frames.get_frame(img_ID);
		results.push_back(Json::object{
		  { "video", int(vf.video_ID + 1) },
		  { "frame", int(vf.frame_number) },
		  { "score", double(scores[img_ID]) },
		});
	}

	std::vector<Json> used_cats;
	std::vector<Json> used_types;
	std::vector<Json> sort_types;

	std::string query_val(sentence_query + ";");

	// If Top KNN request
	if (used_tools.topknn_used) {

		// Mark this as KNN request
		query_val += "show_knn;";

		used_cats.push_back("image");
		used_types.push_back("feedbackModel");
		sort_types.push_back("feedbackModel");
	}
	// Else normal rescore
	else {

		// Just mark that this was NOT KNN request
		query_val += "normal_rescore;";

		if (used_tools.KWs_used) {
			used_cats.push_back("text");
			used_types.push_back("jointEmbedding");
			sort_types.push_back("jointEmbedding");
		}

		if (used_tools.bayes_used) {
			used_cats.push_back("image");
			used_types.push_back("feedbackModel");
			sort_types.push_back("feedbackModel");
		}
	}

	query_val += "from_video_limit=";
	query_val += std::to_string(topn_frames_per_video);
	query_val += ";from_shot_limit=";
	query_val += std::to_string(topn_frames_per_shot);

	Json result_json_arr = Json::array(results);

	Json top = Json::object{ { "teamId", int(cfg.team_ID) },
		                 { "memberId", int(cfg.member_ID) },
		                 { "timestamp", double(timestamp()) },
		                 { "usedCategories", used_cats },
		                 { "usedTypes", used_types },
		                 { "sortType", sort_types },
		                 { "resultSetAvailability", "top" },
		                 { "type", "result" },
		                 { "value", query_val },
		                 { "results", std::move(result_json_arr) } };

	start_sender(cfg.submit_rerank_URL, "", top.dump());
}

void
Submitter::log_add_keywords(const std::string &query_sentence)
{
	push_event("text", "jointEmbedding", query_sentence);
}
void
Submitter::log_like(const DatasetFrames &frames,
                    DisplayType /*disp_type*/,
                    ImageId frame_ID)
{
	auto vf = frames.get_frame(frame_ID);

	std::stringstream data_ss;
	data_ss << "VId" << (vf.video_ID + 1) << ",FN" << vf.frame_number
	        << ";FId" << frame_ID << ";like;";

	push_event("image", "feedbackModel", data_ss.str());
}

void
Submitter::log_dislike(const DatasetFrames &frames,
                       DisplayType /*disp_type*/,
                       ImageId frame_ID)
{
	auto vf = frames.get_frame(frame_ID);

	std::stringstream data_ss;
	data_ss << "VId" << (vf.video_ID + 1) << ",FN" << vf.frame_number
	        << ";FId" << frame_ID << ";dislike;";

	push_event("image", "feedbackModel", data_ss.str());
}

void
Submitter::log_show_random_display(const DatasetFrames & /*frames*/,
                                   const std::vector<ImageId> & /*imgs*/)
{
	push_event("browsing", "randomSelection", "random_display;");
}

void
Submitter::log_show_som_display(const DatasetFrames & /*frames*/,
                                const std::vector<ImageId> & /*imgs*/)
{
	push_event("browsing", "exploration", "som_display");
}

void
Submitter::log_show_topn_display(const DatasetFrames & /*frames*/,
                                 const std::vector<ImageId> & /*imgs*/)
{
	push_event("browsing", "rankedList", "topn_display");
}

void
Submitter::log_show_topn_context_display(const DatasetFrames & /*frames*/,
                                         const std::vector<ImageId> & /*imgs*/)
{
	push_event("browsing", "rankedList", "topn_context_display;");
}

void
Submitter::log_show_topknn_display(const DatasetFrames &frames,
                                   ImageId frame_ID,
                                   const std::vector<ImageId> & /*imgs*/)
{
	auto vf = frames.get_frame(frame_ID);

	std::stringstream data_ss;
	data_ss << "VId" << (vf.video_ID + 1) << ",FN" << vf.frame_number
	        << ";FId" << frame_ID << ";topknn_display;";

	push_event("image", "globalFeatures", data_ss.str());
}

void
Submitter::log_show_detail_display(const DatasetFrames &frames,
                                   ImageId frame_ID)
{
	auto vf = frames.get_frame(frame_ID);

	std::stringstream data_ss;
	data_ss << "VId" << (vf.video_ID + 1) << ",FN" << vf.frame_number
	        << ";FId" << frame_ID << ";video_detail;";

	push_event("browsing", "videoSummary", data_ss.str());
}

void
Submitter::log_show_video_replay(const DatasetFrames &frames, ImageId frame_ID)
{
	static int64_t last_replay_submit = 0;
	static ImageId last_frame_ID = IMAGE_ID_ERR_VAL;

	// If no need to log now

	if (last_replay_submit + cfg.log_replay_timeout > size_t(timestamp()) &&
	    frame_ID == last_frame_ID)
		return;

	last_replay_submit = timestamp();
	last_frame_ID = frame_ID;

	auto vf = frames.get_frame(frame_ID);

	std::stringstream data_ss;
	data_ss << "VId" << (vf.video_ID + 1) << ",FN" << vf.frame_number
	        << ";FId" << frame_ID << ";replay;";

	push_event("browsing", "temporalContext", data_ss.str());
}

void
Submitter::log_scroll(const DatasetFrames & /*frames*/,
                      DisplayType from_disp_type,
                      float dirY)
{
	std::string ev_type("rankedList");
	std::string disp_type;

	switch (from_disp_type) {
		case DisplayType::DTopN:
			ev_type = "rankedList";
			disp_type = "topn_display";
			break;

		case DisplayType::DTopNContext:
			ev_type = "rankedList";
			disp_type = "topn_display_with_context";
			break;

		case DisplayType::DTopKNN:
			ev_type = "rankedList";
			disp_type = "topknn_display";
			break;

		case DisplayType::DVideoDetail:
			ev_type = "videoSummary";
			disp_type = "video_detail";
			break;

		default:
			return;
			break;
	}

	static int64_t last_logged = 0;
	static DisplayType last_disp_type = DisplayType::DNull;

	// If no need to log now

	if (last_logged + cfg.log_replay_timeout > size_t(timestamp()) &&
	    from_disp_type == last_disp_type)
		return;

	last_logged = timestamp();
	last_disp_type = from_disp_type;

	std::stringstream data_ss;
	data_ss << "scroll" << (dirY > 0 ? "Up" : "Down") << ";" << dirY << ";"
	        << disp_type << ";";

	push_event("browsing", ev_type, data_ss.str());
}

void
Submitter::log_reset_search()
{
	push_event("browsing", "resetAll", "");
}

void
Submitter::start_sender(const std::string &submit_url,
                        const std::string &query_string,
                        const std::string &post_data)
{
	finish_flags.emplace_back(std::make_unique<bool>(false));
	submit_threads.emplace_back(submitter_thread,
	                            submit_url,
	                            query_string,
	                            post_data,
	                            std::ref(*(finish_flags.back())),
	                            cfg);
}

void
Submitter::poll()
{
	if (last_submit_timestamp + cfg.send_logs_to_server_period <
	    size_t(timestamp()))
		send_backlog_only();

	for (size_t i = 0; i < submit_threads.size();)
		if (*finish_flags[i]) {
			submit_threads[i].join();
			submit_threads.erase(submit_threads.begin() + i);
			finish_flags.erase(finish_flags.begin() + i);
		} else
			++i;
}

void
Submitter::send_query_with_backlog(const std::string &query_string)
{

	if (!backlog.empty()) {
		Json a = Json::object{ { "timestamp", double(timestamp()) },
			               { "events", std::move(backlog) },
			               { "type", "interaction" } };
		backlog.clear();
		start_sender(cfg.submit_URL, query_string, a.dump());
	} else if (!query_string.empty())
		start_sender(cfg.submit_URL, query_string, "");

	// We always reset timer
	last_submit_timestamp = timestamp();
}

void
Submitter::push_event(const std::string &cat,
                      const std::string &type,
                      const std::string &value)
{
	Json a = Json::object{ { "teamId", int(cfg.team_ID) },
		               { "memberId", int(cfg.member_ID) },
		               { "timestamp", double(timestamp()) },
		               { "category", cat },
		               { "type", type },
		               { "value", value } };

	backlog.emplace_back(std::move(a));
}
