#pragma once

/** @file */

#include "move.hpp"
#include <set>
#include <vector>

namespace chess
{
	struct MoveTreeNode
	{
		std::vector<MoveTreeNode> responses{};
		RatedMove move{};
		Color move_played_by{};
		size_t hash = 0;

		void evaluate_next(const Board& _previousBoard);

		size_t tree_size() const;
		size_t total_outcomes() const;

		MoveTreeNode() = default;
	};

	constexpr auto q = sizeof(MoveTreeNode);
	



	struct MoveTree
	{
		chess::Board board{}; // initial board state
		std::vector<MoveTreeNode> moves{}; // moves that can be played from the initial board state
		Color to_play{}; // who is to play

		void evalulate_next();
		std::optional<Move> best_move();
		size_t tree_size() const;
		size_t total_outcomes() const;

		MoveTree() = default;
	};


};