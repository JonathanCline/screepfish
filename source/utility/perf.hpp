#pragma once

/** @file */

#include <chrono>
#include <thread>
#include <vector>
#include <numeric>
#include <algorithm>

namespace sch
{
	inline auto average_runtime(auto&& _op, size_t _times)
	{
		namespace ch = std::chrono;
		using clk = ch::steady_clock;
		using dur = ch::duration<double>;

		auto _runs = std::vector<dur>(_times, dur{});

		for (size_t n = 0; n != _times; ++n)
		{
			const auto t0 = clk::now();
			_op();
			const auto t1 = clk::now();
			_runs[n] = ch::duration_cast<dur>(t1 - t0);
			std::this_thread::yield();
		};

		dur _avgDur = std::accumulate(_runs.begin(), _runs.end(), dur{});
		return _avgDur / _runs.size();
	};

	inline size_t count_runs_within_duration(auto&& _op, std::chrono::nanoseconds _maxTime)
	{
		namespace ch = std::chrono;
		using clk = ch::steady_clock;

		size_t n = 0;
		const auto tEnd = clk::now() + _maxTime;
		do
		{
			_op();
			++n;
		} while (clk::now() < tEnd);

		return n;
	};

};
