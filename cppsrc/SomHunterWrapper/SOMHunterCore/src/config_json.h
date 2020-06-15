
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
#ifndef CONFIG_JSON_H_
#define CONFIG_JSON_H_

#include <fstream>
#include <stdexcept>
#include <string>

#include "json11.hpp"

#include "log.h"
#include "utils.h"

struct VideoFilenameOffsets
{
	size_t filename_off;
	size_t vid_ID_off;
	size_t vid_ID_len;
	size_t shot_ID_off;
	size_t shot_ID_len;
	size_t frame_num_off;
	size_t frame_num_len;
};

struct SubmitterConfig
{
	/** If submit actions will create actual HTTP request */
	bool submit_to_VBS;
	std::string submit_rerank_URL;
	std::string submit_URL;

	size_t team_ID;
	size_t member_ID;

	std::string VBS_submit_archive_dir;
	std::string VBS_submit_archive_log_suffix;
	bool extra_verbose_log;

	size_t send_logs_to_server_period;
	size_t log_replay_timeout;
};

/** Parsed current config of the core.
 * \see ParseJsonConfig
 */
struct Config
{
	SubmitterConfig submitter_config;

	size_t max_frame_filename_len;
	VideoFilenameOffsets filename_offsets;

	std::string frames_list_file;
	std::string frames_path_prefix;

	size_t features_file_data_off;
	std::string features_file;
	size_t features_dim;

	size_t pre_PCA_features_dim;
	std::string kw_bias_vec_file;
	std::string kw_scores_mat_file;
	std::string kw_PCA_mean_vec_file;
	std::string kw_PCA_mat_file;
	size_t kw_PCA_mat_dim;

	std::string kws_file;

	size_t display_page_size;
	size_t topn_frames_per_video;
	size_t topn_frames_per_shot;

	static Config parse_json_config(const std::string &filepath);
};

/** Parsees the JSON config file that holds initial config.
 *
 * That means basically what we have in config.h now (paths etc.)
 */
inline Config
Config::parse_json_config(const std::string &filepath)
{
	std::ifstream ifs{ filepath };
	if (!ifs.is_open()) {
		std::string msg{ "Error opening file: " + filepath };
		warn(msg);
		throw std::runtime_error(msg);
	}

	// Rad the whole file
	ifs.seekg(0, std::ios::end);
	size_t size = ifs.tellg();

	std::string cfg_file_contents(size, ' ');

	ifs.seekg(0);
	ifs.read(&cfg_file_contents[0], size);

	std::string err;
	auto json{ json11::Json::parse(cfg_file_contents, err) };

	if (!err.empty()) {
		std::string msg{ "Error parsing JSON config file: " +
			         filepath };
		warn(msg);
		throw std::runtime_error(msg);
	}

	auto cfg = Config{
		SubmitterConfig{
		  json["submitter_config"]["submit_to_VBS"].bool_value(),
		  json["submitter_config"]["submit_rerank_URL"].string_value(),
		  json["submitter_config"]["submit_URL"].string_value(),

		  size_t(json["submitter_config"]["team_ID"].int_value()),
		  size_t(json["submitter_config"]["member_ID"].int_value()),

		  json["submitter_config"]["VBS_submit_archive_dir"]
		    .string_value(),
		  json["submitter_config"]["VBS_submit_archive_log_suffix"]
		    .string_value(),
		  json["submitter_config"]["extra_verbose_log"].bool_value(),

		  size_t(json["submitter_config"]["send_logs_to_server_period"]
		           .int_value()),
		  size_t(
		    json["submitter_config"]["log_replay_timeout"].int_value()),
		},

		size_t(json["max_frame_filename_len"].int_value()),

		VideoFilenameOffsets{
		  size_t(
		    json["filename_offsets"]["fr_filename_off"].int_value()),
		  size_t(json["filename_offsets"]["fr_filename_vid_ID_off"]
		           .int_value()),
		  size_t(json["filename_offsets"]["fr_filename_vid_ID_len"]
		           .int_value()),
		  size_t(json["filename_offsets"]["fr_filename_shot_ID_off"]
		           .int_value()),
		  size_t(json["filename_offsets"]["fr_filename_shot_ID_len"]
		           .int_value()),
		  size_t(json["filename_offsets"]["fr_filename_frame_num_off"]
		           .int_value()),
		  size_t(json["filename_offsets"]["fr_filename_frame_num_len"]
		           .int_value()),
		},

		json["frames_list_file"].string_value(),
		json["frames_path_prefix"].string_value(),

		size_t(json["features_file_data_off"].int_value()),
		json["features_file"].string_value(),
		size_t(json["features_dim"].int_value()),

		size_t(json["pre_PCA_features_dim"].int_value()),
		json["kw_bias_vec_file"].string_value(),
		json["kw_scores_mat_file"].string_value(),
		json["kw_PCA_mean_vec_file"].string_value(),
		json["kw_PCA_mat_file"].string_value(),
		size_t(json["kw_PCA_mat_dim"].int_value()),

		json["kws_file"].string_value(),

		size_t(json["display_page_size"].int_value()),
		size_t(json["topn_frames_per_video"].int_value()),
		size_t(json["topn_frames_per_shot"].int_value()),
	};

	return cfg;
}

#endif // CONFIG_JSON_H_