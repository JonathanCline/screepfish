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
	{
		namespace tm = sch::tm;
		
		auto _terminal = tm::Terminal();
		_terminal.open();



		auto _btn = _terminal.add_button(2, 2, 8, 8);
		_btn->label_ = "Test";
		_btn->draw();
		_btn->on_mouse_event_ = [&_terminal](tm::Button& _button, int _mouseButton, int _state)
		{
			if (_state == 0)
			{
				_button.pressed_ = false;
			}
			else
			{
				_button.pressed_ = true;
			};
			_button.draw();
		};
		

		while (!_terminal.should_close())
		{
			_terminal.update();


			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		};

		_terminal.close();

		return 0;
	}

	
	bool _perf = false;
	bool _local = false;
	bool _tests = true;

	for (const auto& _varg : _vargs)
	{
		const auto _arg = std::string_view(_varg);
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

	if (_tests && !sch::run_tests_main())
	{
		return 1;
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