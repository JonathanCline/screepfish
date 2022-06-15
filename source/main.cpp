#include "screepfish.hpp"

#include "env.hpp"

#include "nn/net.hpp"

#include "terminal/terminal.hpp"

#include "chess/fen.hpp"
#include "chess/chess.hpp"
#include "chess/board.hpp"
#include "chess/move_tree.hpp"

#include <fstream>

// Add the custom executable arguments header if present
#if __has_include("_exec_args.hpp")
	#include "_exec_args.hpp"
#endif

#include "utility/logging.hpp"
#include "utility/utility.hpp"

#include <jclib/cli.hpp>
#include <span>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <thread>

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



inline auto make_chess_board_nn_inputs(const chess::Board& _board)
{
	std::array<nn::SimpleNeuron::value_type, 64> _inputValues{};
	for (auto& _pos : chess::positions_v)
	{
		const auto _piece = _board.get(_pos);
		if (_piece)
		{
			const auto _sig = chess::piece_signature_value(_piece.type());
			_inputValues.at(static_cast<size_t>(_pos)) = nn::sigmoid(
				(_piece.color() == chess::Color::white) ? _sig : -_sig
			);
		};
	};
	return _inputValues;
};



int rmain(std::span<const char* const> _vargs)
{
	bool _perf = false;
	bool _local = false;
	bool _tests = false;

	if (_vargs.size() <= 1)
	{
		sch::log_error("Missing arguments, use -h or --help to print usage");
	}
	
	{
		const auto _argCStr = _vargs[1];
		const auto _arg = std::string_view(_argCStr);

		if (_arg == "--help" || _arg == "-h")
		{
			std::cout << "screepfish [<mode> [args...]]\n";
			std::cout <<
				" Arguments:\n"
				"  <mode> = { perf | test | local | {--h|--help} }\n" <<
				"    {--h|--help} : Prints this message\n" <<
				"    perf : Runs the performance test\n" <<
				"    test : Runs the tests\n" <<
				"    local : Plays a local game\n" <<
				" If no <mode> is provided then this connects to lichess\n\n";
			return 0;
		}
		else if (_arg == "perf")
		{
			_perf = true;
		}
		else if (_arg == "test")
		{
			_tests = true;
		}
		else if (_arg == "local")
		{
			_local = true;
		}
		else
		{
			auto _modeStrings = std::array<std::string_view, 5>{ "perf", "local", "test", "-h", "--help" };
			auto it = str::find_longest_match(_modeStrings, _arg);

			auto s = "Urecognized mode \"" + std::string(_arg) + "\"";
			if (it != _modeStrings.end())
			{
				s = s + " (closest match \"" + std::string(*it) + "\")";
			};
			s += ", Use -h or --help to print usage";
			sch::log_error(s);

			return 1;
		};
	};

	sch::log_output_chunk_divider();

	if (_perf)
	{
		sch::perf_test();
		exit(0);
	}
	else if (_tests)
	{
		if (!sch::run_tests_main())
		{
			exit(1);
		};
		exit(0);
	};

	if (_local)
	{
		return sch::local_game_main((int)_vargs.size(), _vargs.data());
	}
	else
	{
		return sch::lichess_bot_main((int)_vargs.size(), _vargs.data());
	};
};



int main(int _nargs, const char* const* _vargs)
{
	std::span<const char* const> _args{};

#ifdef _SCREEPFISH_EXEC_ARGS
	const auto _customArgs = sch::prepend_array(executable_args, _vargs[0]);
	_args = std::span<const char* const>(_customArgs);
#else
	_args = std::span<const char* const>(_vargs, _nargs);
#endif

	return rmain(_args);
};