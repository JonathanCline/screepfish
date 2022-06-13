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
		bool enable_pruning_ = false;

		MoveTreeProfile() = default;
	};




	struct MoveTreeAlphaBeta
	{
		Rating alpha;
		Rating beta;

		MoveTreeAlphaBeta() = default;
	};


	/**
	 * @brief Extra info carried around during the move search part of the move tree evaluation.
	*/
	struct MoveTreeSearchData
	{
		uint8_t depth_ = 0;
		uint8_t max_depth_ = 255;




		MoveTreeSearchData with_next_depth() const
		{
			auto o = MoveTreeSearchData{ *this };
			++o.depth_;
			return o;
		};

		bool can_go_deeper() const
		{
			if (this->depth_ + 1 < this->max_depth_)
			{
				return true;
			}
			else
			{
				return false;
			};
		};

		bool try_going_deeper()
		{
			if (this->can_go_deeper())
			{
				++this->depth_;
				return true;
			}
			else
			{
				return false;
			};
		};

		constexpr MoveTreeSearchData() = default;
		constexpr explicit MoveTreeSearchData(uint8_t _maxDepth) noexcept :
			max_depth_(_maxDepth)
		{};
	};


	struct NodeEvalResult
	{
		bool follow_check_ = false;
		bool follow_capture_ = false;

		NodeEvalResult() = default;
	};

	struct MoveTreeNode
	{
	public:

		void resort_children();

		using size_type = uint8_t;

		/**
		 * @brief Gets the rating for the position.
		 * @return Absolute rating.
		*/
		Rating rating() const
		{
			return this->rating_;
		};

		/**
		 * @brief Gets the rating for the position.
		 * @param _player Player to get the rating's value for.
		 * @return Rating for the given player.
		*/
		//Rating rating(Color _player) const
		//{
		//	return this->rating().player(_player);
		//};

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
		
		NodeEvalResult evaluate_next(const Board& _previousBoard,
			const MoveTreeProfile& _profile, MoveTreeSearchData _data);


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
		Rating rating_ = 0_rt;
	public:
	};


	/**
	 * @brief Evaluates the next full move for a given move node.
	 * 
	 * @param _board Board prior to playing the given node's move.
	 * @param _node Node to evaluate next full move for.
	 * @param _profile Tree profile configuring search/eval handling.
	 * @param _searchData Extra data about the search.
	*/
	void evaluate_next_full_move(const Board& _board, MoveTreeNode& _node,
		const MoveTreeProfile& _profile, MoveTreeSearchData _searchData);




	struct MoveTree
	{
	private:
		void resort_children(std::mt19937* _rnd);

	public:
		// Common evaluate next function
		void evaluate_next(MoveTreeSearchData _searchData, const MoveTreeProfile& _profile = MoveTreeProfile());
		void evalulate_next(MoveTreeSearchData _searchData, const MoveTreeProfile& _profile = MoveTreeProfile());

		std::optional<RatedMove> best_move(std::mt19937& _rnd);
		size_t tree_size() const;
		size_t total_outcomes() const;

		std::vector<std::vector<RatedMove>> get_top_lines(size_t _maxCount) const;

		size_t count_unique_positions();
		size_t count_checks();

		void build_tree(size_t _depth, size_t _maxExtendedDepth, const MoveTreeProfile& _profile = MoveTreeProfile());
		void build_tree(size_t _depth, const MoveTreeProfile& _profile = MoveTreeProfile())
		{
			return this->build_tree(_depth, _depth + 2, _profile);
		};

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