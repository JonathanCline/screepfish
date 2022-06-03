#pragma once

/** @file */

namespace sch
{
	bool run_tests();

	void perf_test();

	bool local_game(const char* _assetsDirectoryPath);


	int local_game_main(int _nargs, const char* _vargs[]);
	int lichess_bot_main(int _nargs, const char* _vargs[]);

};
