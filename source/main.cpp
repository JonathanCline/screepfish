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



	if (_nargs >= 2 && _vargs[1] == std::string_view("--perf"))
	{
		sch::perf_test();
		exit(0);
	};

	if (!sch::run_tests())
	{
		return 1;
	};

	return sch::local_game_main(_nargs, _vargs);
};