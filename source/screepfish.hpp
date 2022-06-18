#pragma once

/** @file */

#include "utility/utility.hpp"


#include <string>
#include <span>

namespace sch
{
	/**
	 * @brief Subprogram function type alias.
	*/
	struct SubprogramArgs
	{
	private:
		using cont = std::span<const char* const>;
	public:

		bool empty() const { return this->args_.empty(); };
		auto size() const { return this->args_.size(); };

		auto data() const { return this->args_.data(); };

		auto& invoke_path() const { return this->invoke_path_; };

		auto begin() const { return this->args_.begin(); };
		auto end() const { return this->args_.end(); };

		auto& at(size_t n) const
		{
			SCREEPFISH_CHECK(n < this->args_.size());
			return this->args_[n];
		};
		auto& operator[](size_t n) const
		{
			return this->at(n);
		};

		SubprogramArgs(const std::string& _invokePath, cont _args) :
			invoke_path_(_invokePath), args_(_args)
		{}

	private:
		std::string invoke_path_;
		cont args_;
	};

	/**
	 * @brief Type returned by subprograms when run.
	*/
	using SubprogramResult = int;


	bool run_tests_main();
	int run_tests_subprogram(SubprogramArgs _args);

	void perf_test();
	int perf_test_subprogram(SubprogramArgs _args);

	bool local_game(const char* _assetsDirectoryPath, bool _step);


	int local_game_subprogram(SubprogramArgs _args);
	int lichess_bot_subprogram(SubprogramArgs _args);
	int perft_subprogram(SubprogramArgs _args);
	int moves_subprogram(SubprogramArgs _args);
};
