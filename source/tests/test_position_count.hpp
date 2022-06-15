#pragma once

/** @file */

#include "test_base.hpp"

#include "chess/fen.hpp"
#include "chess/chess.hpp"
#include "chess/move.hpp"
#include "chess/move_tree.hpp"

#include <vector>
#include <string_view>


namespace sch
{
	class Test_PositionCount : public ITest
	{
	public:

		TestResult run() final
		{
			const auto _searchData = chess::MoveTreeSearchData();
			auto _tree = chess::MoveTree(this->board_);
			
			auto _profile = chess::MoveTreeProfile();
			_profile.alphabeta_ = false;
			_profile.enable_pruning_ = false;
			_profile.follow_captures_ = false;
			_profile.follow_checks_ = false;

			for (auto& v : this->expected_)
			{
				_tree.evaluate_next(_searchData, _profile);
				const auto u = chess::count_final_positions(_tree.initial_board(), _tree.root());
				if (u != v)
				{
					const auto _captures =
						chess::count_final_captures(_tree.initial_board(), _tree.root());
					const auto _checks =
						chess::count_final_checks(_tree.initial_board(), _tree.root());
					const auto _doubleChecks =
						chess::count_final_double_checks(_tree.initial_board(), _tree.root());
					const auto _checkmates =
						chess::count_final_checkmates(_tree.initial_board(), _tree.root());
					const auto _castles =
						chess::count_final_castles(_tree.initial_board(), _tree.root());
					const auto _enpassants =
						chess::count_final_enpassants(_tree.initial_board(), _tree.root());

					const auto s =
						"Expected " + std::to_string(v) +
						" positions - got " + std::to_string(u) +
						"\n fen = " + chess::get_fen(_tree.initial_board()) +
						"\n delta = " + std::to_string((int)v - (int)u) +
						"\n   positions     = " + std::to_string(u) +
						"\n   captures      = " + std::to_string(_captures) +
						"\n   checks        = " + std::to_string(_checks) +
						"\n   double checks = " + std::to_string(_doubleChecks) +
						"\n   checkmates    = " + std::to_string(_checkmates) +
						"\n   castles       = " + std::to_string(_castles) +
						"\n   enpassants    = " + std::to_string(_enpassants);

					return TestResult(this->name_, -1, s);
				};
			};

			return TestResult(this->name_);
		};

		Test_PositionCount(std::string_view _name, chess::Board _board, std::vector<size_t> _expectedPositions) :
			name_(_name), board_(_board),
			expected_(_expectedPositions)
		{};

	private:
		std::string name_;
		chess::Board board_;
		std::vector<size_t> expected_;
	};
};