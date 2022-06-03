#pragma once

/** @file */

#include "move.hpp"

#include <set>
#include <vector>
#include <random>

namespace chess
{


	struct MoveTreeNode
	{
		Rating rating() const { return this->rating_; };
		Rating quick_rating() const { return this->move.rating(); };


		std::vector<MoveTreeNode> responses{};
		RatedMove move{};
		Rating rating_ = 0;

		void evaluate_next(const Board& _previousBoard, bool _followChecks = true);

		size_t tree_size() const;
		size_t total_outcomes() const;

		void show_best_line() const;
		std::vector<RatedMove> get_best_line() const;

		MoveTreeNode() = default;
	};




	struct MoveTree
	{
		chess::Board board{}; // initial board state
		std::vector<MoveTreeNode> moves{}; // moves that can be played from the initial board state

		void evalulate_next();
		std::optional<Move> best_move(std::mt19937& _rnd);
		size_t tree_size() const;
		size_t total_outcomes() const;

		std::vector<std::vector<RatedMove>> get_top_lines(size_t _maxCount) const;

		MoveTree() = default;

		size_t depth_counter_ = 0;
	};


};