
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

#ifndef image_path_h
#define image_path_h

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#include "common.h"
#include "config_json.h"
#include "utils.h"

struct VideoFrame;

using VideoFramePointer = const VideoFrame *;

struct VideoFrame
{
	inline VideoFrame(std::string &&filename,
	                  VideoId video_ID,
	                  ShotId shot_ID,
	                  ImageId frame_number,
	                  ImageId image_ID)
	  : filename(std::move(filename))
	  , video_ID(video_ID)
	  , shot_ID(shot_ID)
	  , frame_number(frame_number)
	  , frame_ID(image_ID)
	{}

	std::string filename;
	VideoId video_ID;
	ShotId shot_ID;
	ImageId frame_number;
	ImageId frame_ID;

	bool liked{ false };
};

/**
 * Represents CONTINOUS range of frames.
 */
struct FrameRange
{
	std::vector<VideoFrame>::iterator _begin;
	std::vector<VideoFrame>::iterator _end;

	FrameRange() = default;
	FrameRange(std::vector<VideoFrame>::iterator b,
	           std::vector<VideoFrame>::iterator e)
	  : _begin(b)
	  , _end(e)
	{}

	size_t size() const { return _end - _begin; }
	/**
	 * Returns VideoFrame reference to the frame with given index
	 * in this frame range.
	 */
	const VideoFrame &operator[](size_t idx) const
	{
		// Iterator is random access so this is fine
		return *(_begin + idx);
	}
	VideoFrame &operator[](size_t idx)
	{
		// Iterator is random access so this is fine
		return *(_begin + idx);
	}

	std::vector<VideoFrame>::const_iterator begin() const { return _begin; }
	std::vector<VideoFrame>::iterator begin() { return _begin; }

	std::vector<VideoFrame>::const_iterator end() const { return _end; }
	std::vector<VideoFrame>::iterator end() { return _end; }
};

/**
 * Represents CONTINOUS range of const frame pointers.
 */
class FramePointerRange
{
	std::vector<VideoFramePointer>::const_iterator _begin;
	std::vector<VideoFramePointer>::const_iterator _end;
	bool _valid{ false };

public:
	FramePointerRange() = default;
	FramePointerRange(std::vector<VideoFramePointer>::const_iterator b,
	                  std::vector<VideoFramePointer>::const_iterator e)
	  : _begin(b)
	  , _end(e)
	  , _valid(true)
	{}
	FramePointerRange(const std::vector<VideoFramePointer> &v)
	  : _begin(v.cbegin())
	  , _end(v.cend())
	  , _valid(true)
	{}

	/**
	 * Returns true iff FramePointerRange contains valid range
	 * at the time. Otherwise iterators behaviour is undefined.
	 */
	bool valid() const { return _valid; }

	size_t size() const { return _end - _begin; }
	/**
	 * Returns VideoFramePointer reference to the frame with given index
	 * in this frame range.
	 */
	const VideoFramePointer &operator[](size_t idx) const
	{
		// Iterator is random access so this is fine
		return *(_begin + idx);
	}
	const VideoFramePointer &operator[](size_t idx)
	{
		// Iterator is random access so this is fine
		return *(_begin + idx);
	}

	std::vector<VideoFramePointer>::const_iterator begin() const
	{
		return _begin;
	}

	std::vector<VideoFramePointer>::const_iterator end() const
	{
		return _end;
	}
};

class DatasetFrames
{
	/** Map from video ID to range of image IDs */
	std::vector<FrameRange> _video_ID_to_frame_range;
	std::vector<VideoFrame> _frames;

	std::string frames_path_prefix;
	VideoFilenameOffsets offs{};

public:
	DatasetFrames(const Config &config);

	std::string operator[](ImageId i) const
	{
		return frames_path_prefix +
		       std::string{ _frames.at(i).filename };
	}

	std::vector<VideoFrame>::iterator end() { return _frames.end(); };
	std::vector<VideoFrame>::iterator begin() { return _frames.begin(); };

	std::vector<VideoFrame>::const_iterator end() const
	{
		return _frames.end();
	};
	std::vector<VideoFrame>::const_iterator begin() const
	{
		return _frames.begin();
	};

	size_t get_num_videos() const { return _frames.back().video_ID + 1; }

	VideoFrame &get_frame(ImageId i) { return _frames[i]; }

	const VideoFrame &get_frame(ImageId i) const { return _frames[i]; }

	std::vector<VideoFrame>::const_iterator get_frame_it(ImageId i) const
	{
		return _frames.begin() + i;
	}

	size_t size() const { return _frames.size(); }

	VideoId get_video_id(ImageId img_ID) const
	{
		if (img_ID >= _frames.size()) {
			return VIDEO_ID_ERR_VAL;
		} else {
			return get_frame(img_ID).video_ID;
		}
	}

	/**
	 * Return copy of FrameRange representing all selected frames from
	 * the given video.
	 */
	FrameRange get_all_video_frames(VideoId video_ID) const
	{
		return _video_ID_to_frame_range[video_ID];
	}

	/**
	 * Returns new instance of FrameRange representing all frames from
	 * prvided video ID in interval [frame_num_from, frame_num_to]
	 */
	FrameRange get_shot_frames(VideoId video_ID,
	                           size_t frame_num_from,
	                           size_t frame_num_to) const
	{
		// Get video range
		auto video_range = _video_ID_to_frame_range[video_ID];

		auto from_it = video_range.begin();
		auto to_it = video_range.end();

		--to_it;

		// Rewind to the first wanted frame
		while (from_it->frame_number < frame_num_from) {
			++from_it;
		}

		// Rewind to the last wanted frame
		while (to_it->frame_number > frame_num_to) {
			--to_it;
		}

		// Return one frame behind to behave begin/end-like
		return FrameRange(from_it, to_it + 1);
	}

	/** Translation to VideoFrameRefs from vector ids or FrameRange */
	std::vector<VideoFramePointer> ids_to_video_frame(
	  const std::vector<ImageId> &ids) const;
	static std::vector<VideoFramePointer> range_to_video_frame(
	  const FrameRange &ids);

private:
	/**
	 * Parses a text file  with lists of top keywords for given image ID.
	 *    e.g:  `with_top.images_top_keywords.txt`
	 *
	 * Line determines image ID therefore input file must have them sorted.
	 */
	static std::vector<std::vector<KeywordId>>
	parse_top_kws_for_imgs_text_file(const std::string &filepath);

	/**
	 * From filename string it parses useful info as video/shot/frame ID
	 * etc.
	 */
	VideoFrame parse_video_filename(std::string &&filename);
};

#endif
