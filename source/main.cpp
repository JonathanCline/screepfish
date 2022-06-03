#include "screepfish.hpp"

#include "env.hpp"

#include <jclib/cli.hpp>

#include <cstdlib>
#include <iostream>
#include <filesystem>

/*
	Initial:
		rt : 3079
		rt : 3305
		rt : 3598
		rt : 3613
		rt : 3691
		rt : 3741
		rt : 3680
		rt : 3723
		rt : 3670
		rt : 3874
*/


int main(int _nargs, const char* _vargs[])
{
	auto _parser = jc::ArgumentParser();
	_parser.add_argument("--perf")
		.set_help("Runs performance test");

	auto _args = _parser.parse_args(_nargs, _vargs);

	bool _perf = false;
	bool _local = false;
	bool _tests = true;

	for (int n = 1; n < _nargs; ++n)
	{
		const auto _arg = std::string_view(_vargs[n]);
		if (_arg == "--perf" || _arg == "-p")
		{
			_perf = true;
		}
		else if (_arg == "--notest")
		{
			_tests = false;
		}
		else if (_arg == "--local" || _arg == "-l")
		{
			_local = true;
		};
	};

	if (_perf)
	{
		sch::perf_test();
		exit(0);
	};

	if (_tests && !sch::run_tests())
	{
		return 1;
	};

	if (_local)
	{
		return sch::local_game_main(_nargs, _vargs);
	}
	else
	{
		return sch::lichess_bot_main(_nargs, _vargs);
	};
};