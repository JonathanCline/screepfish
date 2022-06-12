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
			
			
			for (auto& v : this->expected_)
			{
				_tree.evaluate_next(_searchData);
				if (const auto t = _tree.total_outcomes(); t != v)
				{
					const auto u = _tree.count_unique_positions();
					const auto s =
						"Expected " + std::to_string(v) +
						" positions - got " + std::to_string(t) +
						"\n delta = " + std::to_string((int)v - (int)t) +
						"\n unique = " + std::to_string(u) +
						"\n checks = " + std::to_string(_tree.count_checks()) +
						"\n fen = " + chess::get_fen(_tree.initial_board());
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