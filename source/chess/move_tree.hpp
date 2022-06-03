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
		using size_type = uint8_t;

		Rating rating() const { return this->rating_; };
		Rating quick_rating() const { return this->move.rating(); };

		bool empty() const { return this->responses_count_ == 0; };
		void resize(size_type _size)
		{
			this->responses = std::make_unique<MoveTreeNode[]>(_size);
			this->responses_count_ = _size;
		};

		size_type size() const { return this->responses_count_; };

		auto begin() { return this->responses.get(); };
		auto begin() const { return this->responses.get(); };
		auto end() { return this->begin() + this->size(); };
		auto end() const { return this->begin() + this->size(); };

		auto& front()
		{
			return this->responses[0];
		};
		const auto& front() const
		{
			return this->responses[0];
		};

		void count_duplicates(Board _board, std::set<size_t>& _boards);


		std::unique_ptr<MoveTreeNode[]> responses{};
		RatedMove move{};
		Rating rating_ = 0;
		uint8_t responses_count_ = 0;

		void evaluate_next(const Board& _previousBoard, bool _followChecks = true);

		size_t tree_size() const;
		size_t total_outcomes() const;

		size_t count_checks(Board _board);

		void show_best_line() const;
		std::vector<RatedMove> get_best_line() const;

		MoveTreeNode() = default;
	};

	constexpr static auto q = sizeof(MoveTreeNode);




	struct MoveTree
	{
		chess::Board board{}; // initial board state
		std::vector<MoveTreeNode> moves{}; // moves that can be played from the initial board state

		void evalulate_next();
		std::optional<Move> best_move(std::mt19937& _rnd);
		size_t tree_size() const;
		size_t total_outcomes() const;

		std::vector<std::vector<RatedMove>> get_top_lines(size_t _maxCount) const;

		size_t count_unique_positions();
		size_t count_checks();

		MoveTree() = default;

		size_t depth_counter_ = 0;
	};


};