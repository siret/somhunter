
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

#ifndef asyncsom_h
#define asyncsom_h

#include <atomic>
#include <condition_variable>
#include <thread>
#include <vector>

#include "DatasetFeatures.h"
#include "DatasetFrames.h"
#include "RelevanceScores.h"

class AsyncSom
{
	std::thread worker;

	size_t features_dim{};

	// worker sync
	std::condition_variable new_data_wakeup;
	std::mutex worker_lock;

	/*
	 * Worker input protocol:
	 *
	 * new_data is set when a new computation is required after
	 * wakeup to process a new dataset. Worker "eats" this flag together
	 * with input data.
	 *
	 * terminate is set when the worker should exit.
	 */
	bool new_data, terminate;
	std::vector<float> points, scores;

	/*
	 * Worker output protocol:
	 *
	 * m_ready is set when mapping is filled in AND the
	 * memory is fenced correctly.
	 */
	bool m_ready;
	std::vector<std::vector<ImageId>> mapping;

	static void async_som_worker(AsyncSom *parent, const Config &cfg);

public:
	AsyncSom(const Config &cfg);
	~AsyncSom();

	void start_work(const DatasetFeatures &fs, const ScoreModel &sc);

	bool map_ready() const
	{
		bool t = m_ready;
		std::atomic_thread_fence(std::memory_order_acquire);
		return t;
	}
	const std::vector<ImageId> &map(size_t i) const
	{
		return mapping.at(i);
	}
};

#endif
