#include "logging.hpp"

#include "string.hpp"
#include "utility.hpp"

#include <mutex>
#include <atomic>
#include <chrono>
#include <iostream>
#include <charconv>

namespace sch
{
	template <typename T>
	inline bool compare_exchange(std::atomic<T>& _atomic,
		T _ifValue, const T& _newValue)
	{
		return _atomic.compare_exchange_strong(_ifValue, _newValue);
	};


	template <typename T>
	concept cx_writable = requires(std::ostream & ostr, const T & v)
	{
		{ ostr << v } -> std::convertible_to<std::ostream&>;
	};

	struct Logger
	{
		static auto& log_mutex()
		{
			static std::mutex mtx{};
			return mtx;
		};
		static auto acquire_logging_mutex()
		{
			return std::unique_lock(log_mutex());
		};

		template <cx_writable T>
		static void write_one_helper(const T& v)
		{
			std::cout << v;
		};

		template <cx_writable... Ts>
		static void raw_write_log(const std::unique_lock<std::mutex>& _lck, const Ts&... _args)
		{
			(write_one_helper(_args), ...);
		};

		template <cx_writable... Ts>
		static void raw_write_log(const Ts&... _args)
		{
			const auto _lck = std::unique_lock(log_mutex());
			raw_write_log(_lck, _args...);
		};

		static auto& divider_flag()
		{
			static std::atomic<bool> flag_{ false };
			return flag_;
		};
	};

#define DECLROOTTYPE(t) ::std::remove_cvref_t<decltype(t)>

	inline auto get_current_timestamp()
	{
		namespace ch = std::chrono;
		const auto _currentTime = ch::system_clock::now();
		const auto _zonedCurrentTime = ch::zoned_time(ch::current_zone(), _currentTime);
		const auto _localCurrentTime = _zonedCurrentTime.get_local_time();

		const auto _localTimeSinceEpoch = ch::duration_cast<ch::system_clock::duration>(_localCurrentTime.time_since_epoch());
		const auto _localDaysSinceEpoch = ch::duration_cast<ch::days>(_localTimeSinceEpoch);
		const auto _localTimeSinceDay = ch::duration_cast<ch::system_clock::duration>(_localTimeSinceEpoch - ch::duration_cast<ch::nanoseconds>(_localDaysSinceEpoch));
		const auto _localTime = ch::local_time<ch::system_clock::duration>(_localTimeSinceDay);
		const auto _timeParts = ch::hh_mm_ss<DECLROOTTYPE(_localTime)::duration>(_localTime.time_since_epoch());

		const auto h = (long long)_timeParts.hours().count();
		const auto m = (long long)_timeParts.minutes().count();
		const auto s = (long long)_timeParts.seconds().count();
		const auto _sFmt = str::str_format<long long>().set_width_min(2);
		return str::tostr(h) + ":" + str::tostr(m, _sFmt) + ":" + str::tostr(s, _sFmt);
	};


	inline void log_with_category(std::string_view _cat, std::string_view _what)
	{
		const auto lck = Logger::acquire_logging_mutex();
		Logger::divider_flag() = false;
		Logger::raw_write_log(lck, get_current_timestamp(), " [", _cat, "] ", _what, '\n');
	};

	void log_info(std::string_view _what)
	{
		log_with_category("Info", _what);
	};
	void log_warning(std::string_view _what)
	{
		log_with_category("Warning", _what);
	};
	void log_error(std::string_view _what)
	{
		log_with_category("Error", _what);
	};

	void log_output_chunk_divider()
	{
		const auto lck = Logger::acquire_logging_mutex();
		if (!Logger::divider_flag())
		{
			Logger::raw_write_log(lck, str::rep('=', 80), '\n');
			Logger::divider_flag() = true;
		};
	};
	void log_output_chunk(std::string_view _what)
	{
		const auto lck = Logger::acquire_logging_mutex();
		if (!Logger::divider_flag())
		{
			Logger::raw_write_log(lck, str::rep('=', 80), '\n');
			Logger::divider_flag() = true;
		};
		Logger::raw_write_log(lck, _what, '\n', str::rep('=', 80), '\n');
		Logger::divider_flag() = true;
	};

};
