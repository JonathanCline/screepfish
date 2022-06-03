#include "tests.hpp"

#include "test_base.hpp"
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
			std::vector<size_t>{ 20, 400, 8902, 197'281 }
		));
		_tests.push_back(jc::make_unique<Test_PositionCount>
		(
			std::string_view("Position Count - 5"),
			*chess::parse_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"),
			std::vector<size_t>{ 44, 1486, 62'379 }
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
		return _results;
	};
};