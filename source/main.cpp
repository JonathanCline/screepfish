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



/**
 * @brief Subprogram function type alias.
*/
using SubprogramFn = std::function<sch::SubprogramResult(sch::SubprogramArgs)>;

/**
 * @brief Holds a named subprogram.
*/
class Subprogram
{
public:

	void write_help_text(std::ostream& _ostr) const
	{
		_ostr << this->name_ << " : " << this->help_;
	};



	const std::string& name() const noexcept
	{
		return this->name_;
	};
	const std::string& help_text() const noexcept
	{
		return this->help_;
	};

	template <typename... Ts>
	requires (jc::cx_invocable<SubprogramFn, Ts...>)
	sch::SubprogramResult invoke(Ts&&... _args) const
	{
		SCREEPFISH_CHECK(this->fn_ && "Subprogram has no function assigned");
		return std::invoke(this->fn_, std::forward<Ts>(_args)...);
	};

	Subprogram(const std::string& _name, SubprogramFn _fn, const std::string& _help) :
		name_(_name), fn_(std::move(_fn)), help_(_help)
	{};

private:

	/**
	 * @brief The name of the subprogram.
	*/
	std::string name_;

	/**
	 * @brief The help text for the subprogram.
	*/
	std::string help_;

	/**
	 * @brief The function to invoke.
	*/
	SubprogramFn fn_;
};



struct EngineCLI
{
private:

	void print_subprogram_help(std::ostream& _ostr, const Subprogram& _subprogram)
	{
		_ostr << "   ";
		_subprogram.write_help_text(_ostr);
		_ostr << '\n';
	};

public:

	/**
	 * @brief Prints the help message.
	*/
	void print_help(std::ostream& _ostr)
	{
		/*
		screepfish [<mode> [args...]]
		  Arguments:
		    <mode> = { perf | test | local | {--h|--help} }
		      {--h|--help} : Prints this message
		      perf : Runs the performance test
		      perft : Runs perft
		      test : Runs the tests
		      local : Plays a local game
		      lichess : Connects to a lichess account

		 If no <mode> is provided then this connects to lichess
		*/
		
		_ostr << "screepfish <mode> [args...]\n";
		_ostr << "  The greatest chess bot ever made - never beaten by a GM\n\n";
		_ostr << " <mode> :=\n";
		for (const auto& _subprogram : this->programs_)
		{
			this->print_subprogram_help(_ostr, _subprogram);
		};
	};

	/**
	 * @brief Parses command line args and runs the engine.
	 * @param _invokePath Path used to invoke the executable.
	 * @param _vargs The arguments passed after the invoke path.
	*/
	void parse_args(const char* _invokePath, std::span<const char* const> _vargs)
	{
		// Print mini-usage if no arguments are given
		if (_vargs.empty())
		{
			sch::log_error("Missing arguments, use -h or --help to print usage");
			exit(1);
		};

		auto& _ostr = std::cout;

		// Grab subprogram name
		const auto _subprogramNameArg = std::string_view(_vargs[0]);
		
		// Look for help message
		if (_subprogramNameArg == "-h" || _subprogramNameArg == "--help")
		{
			this->print_help(_ostr);
			return;
		};

		// Invoke subprogram
		if (const auto it = std::ranges::find_if(this->programs_, [&_subprogramNameArg](auto& v)
			{
				return v.name() == _subprogramNameArg;
			});
			it != this->programs_.end())
		{
			const auto _result = it->invoke(sch::SubprogramArgs(_invokePath, _vargs));
			exit(_result);
		}
		else
		{
			// Invalid / Unknown subprogram given
			
			// Find closest match
			const auto _names = this->programs_ | std::views::transform([](auto& v) ->
				 const auto&
				{
					return v.name();
				});
			auto _closestIter = str::find_longest_match(_names, _subprogramNameArg);

			_ostr << "Urecognized mode \"" << _subprogramNameArg << "\"";
			if (_closestIter != _names.end())
			{
				_ostr << "\n\tclosest match : \"" << *_closestIter << "\"";
			};
			_ostr << "\n\tUse -h or --help to print usage\n";

			exit(1);
		};
	};




	void add_subprogram(const Subprogram& _subprogram)
	{
		for (auto& v : this->programs_)
		{
			if (v.name() == _subprogram.name())
			{
				SCREEPFISH_CHECK(false && "Redefined subprogram");
			};
		};

		this->programs_.push_back(_subprogram);
	};

	EngineCLI() = default;

private:
	std::vector<Subprogram> programs_;
};



int rmain(const char* _invokePath, std::span<const char* const> _vargs)
{
	auto _engineCLI = EngineCLI();
	{
		_engineCLI.add_subprogram(Subprogram("test", &sch::run_tests_subprogram, "Runs the tests"));
		_engineCLI.add_subprogram(Subprogram("perf", &sch::perf_test_subprogram, "Runs the performance tests"));
		_engineCLI.add_subprogram(Subprogram("lichess", &sch::lichess_bot_subprogram, "Connects to a lichess account and plays games for it"));
		_engineCLI.add_subprogram(Subprogram("positions", &sch::perft_subprogram, "Generator for final positions (basically perft)"));
		_engineCLI.add_subprogram(Subprogram("moves", &sch::moves_subprogram, "Outputs the number of legal moves that can be played from a position"));
		_engineCLI.add_subprogram(Subprogram("local", &sch::local_game_subprogram, "Runs a local game"));
	};
	_engineCLI.parse_args(_invokePath, _vargs);
	return 0;
};

int main(int _nargs, const char* const* _vargs)
{
	std::span<const char* const> _args{};

	if (_nargs == 0)
	{
		sch::log_error("WHAT 0 arguments??? Are you running outside of an OS??!?!??!");
		return 1;
	};

#ifdef _SCREEPFISH_EXEC_ARGS
	const auto _customArgs = sch::prepend_array(executable_args, _vargs[0]);
	_args = std::span<const char* const>(_customArgs);
#else
	_args = std::span<const char* const>(_vargs, _nargs);
#endif

	// Split off the invoke path.
	const auto _invokePath = _args[0];
	_args = _args.last(_args.size() - 1);

	// Invoke the "real main" function.
	return rmain(_invokePath, _args);
};