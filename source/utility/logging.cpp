#include "logging.hpp"

#include "string.hpp"

#include <mutex>
#include <atomic>
#include <iostream>

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
			raw_write_log(_lck, _args, ...);
		};

		static auto& divider_flag()
		{
			static std::atomic<bool> flag_{ false };
			return flag_;
		};
	};



	void log_error(std::string_view _what)
	{
		const auto lck = Logger::acquire_logging_mutex();
		Logger::divider_flag() = false;
		Logger::raw_write_log(lck, "[Error] ", _what, '\n');
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
