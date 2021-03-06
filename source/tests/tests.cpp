#include "tests.hpp"

#include "test_base.hpp"
#include "test_book.hpp"
#include "test_castling.hpp"
#include "test_position_count.hpp"


#include "chess/fen.hpp"

#include <jclib/memory.h>

namespace sch
{

	std::vector<TestResult> run_tests(bool _stopOnFail)
	{
		// Build the test set
		std::vector<std::unique_ptr<ITest>> _tests
		{
			
		};
		
		// Position count tests
		_tests.push_back(jc::make_unique<Test_PositionCount>
		(
			std::string_view("Position Count - Initial"),
			*chess::parse_fen(chess::standard_start_pos_fen_v),
			std::vector<size_t>{ 20, 400, 8902, 197'281, 4'865'609 }
		));
		_tests.push_back(jc::make_unique<Test_PositionCount>
		(
			std::string_view("Position Count - Position 2"),
			*chess::parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "),
			std::vector<size_t>{ 48, 2'039, 97'862, 4'085'603 }
		));
		_tests.push_back(jc::make_unique<Test_PositionCount>
		(
			std::string_view("Position Count - Position 3"),
			*chess::parse_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "),
			std::vector<size_t>{ 14, 191, 2'812, 43'238, 674'624 /*maybe*/ }
		));
		//_tests.push_back(jc::make_unique<Test_PositionCount>
		//(
		//	std::string_view("Position Count - 3"),
		//	*chess::parse_fen(chess::standard_start_pos_fen_v),
		//	std::vector<size_t>{ 20, 400, 8902, 197'281, 4'865'609 }
		//));
		//_tests.push_back(jc::make_unique<Test_PositionCount>
		//(
		//	std::string_view("Position Count - 4"),
		//	*chess::parse_fen(chess::standard_start_pos_fen_v),
		//	std::vector<size_t>{ 20, 400, 8902, 197'281, 4'865'609 }
		//));
		_tests.push_back(jc::make_unique<Test_PositionCount>
		(
			std::string_view("Position Count - Position 5"),
			*chess::parse_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"),
			std::vector<size_t>{ 44, 1486, 62'379 }
		));

		// Castling
		_tests.push_back(jc::make_unique<Test_Castling>
		(
			std::string_view("All Castle"),
			*chess::parse_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1"),
			true, true, true, true
		));
		_tests.push_back(jc::make_unique<Test_Castling>
		(
			std::string_view("No Castle"),
			*chess::parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
			false, false, false, false
		));
		_tests.push_back(jc::make_unique<Test_Castling>
		(
			std::string_view("Castle Through Check"),
			*chess::parse_fen("rnb1kbnr/pppppppp/8/8/8/4q3/PPP1P1PP/R3K2R w KQkq - 0 1"),
			false, false, false, false
		));
		_tests.push_back(jc::make_unique<Test_Castling>
		(
			std::string_view("Castle Out of Check"),
			*chess::parse_fen("rnb1kbnr/pppppppp/8/8/8/6q1/PPP1P1PP/R3K2R w KQkq - 0 1"),
			false, false, false, false
		));




		// Run tests and collect results.
		std::vector<TestResult> _results{};
		for (auto& v : _tests)
		{
			_results.push_back(v->run());
			if (!_results.back() && _stopOnFail)
			{
				break;
			};
		};

		_results.push_back(sch::tests::test_opening_book());

		return _results;
	};
};