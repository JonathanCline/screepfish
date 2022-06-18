#pragma once

/** @file */

namespace sch
{
	bool run_tests_main();

	void perf_test();

	bool local_game(const char* _assetsDirectoryPath, bool _step);


	int local_game_main(int _nargs, const char* const* _vargs);
	int lichess_bot_main(int _nargs, const char* const* _vargs);
	int perft_main(int _nargs, const char* const* _vargs);

	int moves_main(int _nargs, const char* const* _vargs);
};
