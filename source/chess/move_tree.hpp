#pragma once

/** @file */

#include "move.hpp"
#include "rating.hpp"
#include "board_hash.hpp"

#include "utility/bset.hpp"
#include "utility/arena.hpp"

#include <set>
#include <vector>
#include <random>
#include <unordered_set>

namespace chess
{
	
	struct MoveTreeNode;

	namespace impl
	{
		struct MoveTreeNodeBlockAllocator
		{
			using value_type = MoveTreeNode;
			using pointer = value_type*;
			pointer allocate(size_t n) const;
			void deallocate(pointer p) const;
		};
	};


	struct MoveTreeProfile
	{
		bool follow_checks_ = false;
		bool follow_captures_ = false;

		MoveTreeProfile() = default;
	};


	struct MoveTreeNode
	{
	public:

		void resort_children();

		using size_type = uint8_t;

		/**
		 * @brief Gets the full rating for the position - factoring in child nodes.
		 * @return Full rating.
		*/
		Rating rating() const
		{
			return this->rating_;
		};

		/**
		 * @brief Gets the quick rating for the position.
		 * @return Quick rating.
		*/
		Rating quick_rating() const
		{
			return this->move.rating();
		};

		void set_rating(Rating r)
		{
			this->rating_ = r;
		};

		explicit operator bool() const
		{
			return !this->move.is_null();
		};




		bool was_evaluated() const
		{
			return this->responses_.data() != nullptr;
		};
		void mark_as_evaluated()
		{
			if (!this->was_evaluated())
			{
				this->responses_.resize(1, MoveTreeNode{});
				SCREEPFISH_ASSERT(this->responses_.empty());
			};
		};

		bool empty() const
		{
			return this->responses_.empty();
		};

		void resize(size_type _size)
		{
			this->responses_.resize(_size);
		};

		void soft_resize(size_type _size)
		{
			this->responses_.soft_resize(_size);
		};

		size_type size() const
		{
			return this->responses_.size();
		};

		auto begin() { return this->responses_.begin(); };
		auto begin() const { return this->responses_.begin(); };
		auto end() { return this->responses_.end(); };
		auto end() const { return this->responses_.end(); };

		auto& front()
		{
			return this->responses_.front();
		};
		const auto& front() const
		{
			return this->responses_.front();
		};

		void count_duplicates(Board _board, std::set<size_t>& _boards);
		
		void evaluate_next(const Board& _previousBoard, BoardHashSet& _hashSet, const MoveTreeProfile& _profile = MoveTreeProfile());
		void evaluate_next(const Board& _previousBoard, const MoveTreeProfile& _profile = MoveTreeProfile());


		size_t tree_size() const;
		size_t total_outcomes() const;

		size_t count_checks(Board _board);

		void show_best_line() const;
		std::vector<RatedMove> get_best_line() const;

		MoveTreeNode()
		{
			SCREEPFISH_ASSERT(!this->was_evaluated());
		};

	private:
		sch::NullTerminatedArena<MoveTreeNode, impl::MoveTreeNodeBlockAllocator> responses_{};

	public:
		RatedMove move{};
	private:
		Rating rating_ = 0;

	};


	struct MoveTree
	{
	private:

		// Common evaluate next function
		void evaluate_next(BoardHashSet* _hashSet, const MoveTreeProfile& _profile = MoveTreeProfile());

	public:
		void evaluate_next(const MoveTreeProfile& _profile = MoveTreeProfile());
		void evaluate_next_unique(const MoveTreeProfile& _profile = MoveTreeProfile());
		void evalulate_next(const MoveTreeProfile& _profile = MoveTreeProfile());

		std::optional<RatedMove> best_move(std::mt19937& _rnd);
		size_t tree_size() const;
		size_t total_outcomes() const;

		std::vector<std::vector<RatedMove>> get_top_lines(size_t _maxCount) const;

		size_t count_unique_positions();
		size_t count_checks();

		void clear_hashes() { this->hash_set_.clear(); };

		void build_tree(size_t _depth);

		auto begin() { return this->moves_.begin(); };
		auto begin() const { return this->moves_.begin(); };
		auto cbegin() const { return this->moves_.begin(); };
		auto end() { return this->moves_.end(); };
		auto end() const { return this->moves_.end(); };
		auto cend() const { return this->moves_.end(); };

		const chess::Board& initial_board() const
		{
			return this->board_;
		};
		void set_initial_board(const chess::Board& _board)
		{
			this->board_ = _board;
		};


		MoveTree() = default;
		MoveTree(const chess::Board& _board) :
			board_(_board)
		{};

	private:

		chess::Board board_{}; // initial board state
		std::vector<MoveTreeNode> moves_{}; // moves that can be played from the initial board state
		BoardHashSet hash_set_{};
		size_t depth_counter_ = 0;
	};


	/*
		Before:
			midgame (d3_unique) - 38
			midgame (d3) - 48
			opening (d2) - 10531
			midgame (d2) - 1641
	*/

	/*
		After:
			
	*/

};