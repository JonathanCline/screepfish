#pragma once

/** @file */

#include "move.hpp"

#include <vector>

namespace chess
{
	struct MoveTreeNode
	{
		std::vector<MoveTreeNode> responses{};
		RatedMove move{};
		Color move_played_by{};

		void evaluate_next(const Board& _previousBoard);




		MoveTreeNode() = default;
	};

	struct MoveTree
	{
		chess::Board board{}; // initial board state
		std::vector<MoveTreeNode> moves{}; // moves that can be played from the initial board state
		Color to_play{}; // who is to play

		void evalulate_next();
		Move best_move();


		MoveTree() = default;
	};


};