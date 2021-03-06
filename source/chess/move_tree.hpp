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

//#define SCREEPFISH_DEBUG_ALPHABETA

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
		bool alphabeta_ = false;

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

		Color played_by() const noexcept
		{
			return this->player_;
		};

		void resort_children();
		void resort_children_by_quick_rating();

		using size_type = uint8_t;

		/**
		 * @brief Gets the rating for the position.
		 * @return Absolute rating.
		*/
		AbsoluteRating rating() const
		{
			return this->rating_;
		};

		/**
		 * @brief Gets the rating for the position.
		 * @param _player Player to get the rating's value for.
		 * @return Rating for the given player.
		*/
		Rating rating(Color _player) const
		{
			return this->rating().player(_player);
		};

		Rating player_rating() const
		{
			return this->rating(this->played_by());
		};


		/**
		 * @brief Gets the quick rating for the position.
		 * @return Quick rating.
		*/
		Rating quick_rating() const
		{
			return this->move_.rating();
		};

		void set_rating(AbsoluteRating r)
		{
			this->rating_ = r;
		};

		explicit operator bool() const
		{
			return !this->move_.is_null();
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
			return static_cast<size_type>(this->responses_.size());
		};

		auto begin()
		{
			return this->responses_.begin();
		};
		auto begin() const
		{
			return this->responses_.begin();
		};
		auto end()
		{
			return this->responses_.end();
		};
		auto end() const
		{
			return this->responses_.end();
		};

		auto& front()
		{
			return this->responses_.front();
		};
		const auto& front() const
		{
			return this->responses_.front();
		};

		void count_duplicates(Board _board, std::set<size_t>& _boards);
		

		NodeEvalResult evaluate_next_with_board(const Board& _board,
			const MoveTreeProfile& _profile, MoveTreeSearchData _data, bool _autoProp = true);

		NodeEvalResult evaluate_next(const Board& _previousBoard,
			const MoveTreeProfile& _profile, MoveTreeSearchData _data, bool _autoProp = true);


		size_t tree_size() const;
		size_t total_outcomes() const;

		size_t count_checks(Board _board) const;
		



		void show_best_line() const;
		std::vector<const MoveTreeNode*> get_best_line() const;

		size_t best_line_length() const;



		void set_move(RatedMove _move, Color _playedBy)
		{
			this->move_ = _move;
			this->player_ = _playedBy;
			this->rating_ =
				AbsoluteRating(_move.rating() - ((Rating)this->depth_ * 0.01f), _playedBy);
			this->responses_.clear();
		};

		/**
		 * @brief Clears the branches from this node.
		*/
		void clear() noexcept
		{
			this->responses_.clear();
		};


		auto& at(size_type _index)
		{
			SCREEPFISH_ASSERT(_index < this->size());
			return this->responses_.data()[_index];
		};
		const auto& at(size_type _index) const
		{
			SCREEPFISH_ASSERT(_index < this->size());
			return this->responses_.data()[_index];
		};




		bool is_pruned() const noexcept { return this->pruned_; };
		void set_pruned() noexcept { this->pruned_ = true; };


		MoveTreeNode()
		{
			SCREEPFISH_ASSERT(!this->was_evaluated());
		};

	private:
		sch::NullTerminatedArena<MoveTreeNode, impl::MoveTreeNodeBlockAllocator> responses_{};

	public:
		RatedMove move_{};
	private:
		AbsoluteRating rating_ = 0_art;
		
		/**
		 * @brief The player that played this move.
		*/
		Color player_{};
		uint8_t depth_ = 0;
		bool pruned_ = false;
	};
	

	template <jc::cx_invocable<const Board&> T>
	inline void foreach_final_position(const Board& _board, const MoveTreeNode& _node, const T& _op)
	{
		if (_node.empty())
		{
			// If it was evaluated then this is a checkmate.
			if (_node.was_evaluated())
			{
				return;
			}
			else
			{
				_op(_board);
			};
		}
		else
		{
			for (auto& _response : _node)
			{
				auto _nextBoard = _board;
				_nextBoard.move(_response.move_);
				foreach_final_position(_nextBoard, _response, _op);
			};
		};
	};

	template <jc::cx_invocable<const Board&, Move> T>
	inline void foreach_final_move(const Board& _board, const MoveTreeNode& _node, const T& _op)
	{
		if (!_node.empty())
		{
			for (auto& _response : _node)
			{
				if (_response.empty())
				{
					_op(_board, _response.move_);
				}
				else
				{
					auto _nextBoard = _board;
					_nextBoard.move(_response.move_);
					foreach_final_move(_nextBoard, _response, _op);
				};
			};
		};
	};

	template <jc::cx_invocable<const Board&> T>
	inline void foreach_position(const Board& _board, const MoveTreeNode& _node, const T& _op)
	{
		_op(_board);
		for (auto& _response : _node)
		{
			auto _nextBoard = _board;
			_nextBoard.move(_response.move_);
			foreach_position(_nextBoard, _response, _op);
		};
	};




	inline size_t count_checks(const Board& _board, const MoveTreeNode& _node)
	{
		size_t n = 0;
		const auto _op = [&n](const Board& _board)
		{
			if (is_check(_board, Color::white) || is_check(_board, Color::black))
			{
				++n;
			};
		};
		foreach_position(_board, _node, _op);
		return n;
	};
	inline size_t count_positions(const Board& _board, const MoveTreeNode& _node)
	{
		size_t n = 0;
		const auto _op = [&n](const Board& _board)
		{
			++n;
		};
		foreach_position(_board, _node, _op);
		return n;
	};
	
	inline size_t count_final_checks(const Board& _board, const MoveTreeNode& _node)
	{
		size_t n = 0;
		const auto _op = [&n](const Board& _board)
		{
			if (is_check(_board, Color::white) || is_check(_board, Color::black))
			{
				++n;
			};
		};
		foreach_final_position(_board, _node, _op);
		return n;
	};
	inline size_t count_final_positions(const Board& _board, const MoveTreeNode& _node)
	{
		size_t n = 0;
		const auto _op = [&n](const Board& _board)
		{
			++n;
		};
		foreach_final_position(_board, _node, _op);
		return n;
	};
	inline size_t count_final_captures(const Board& _board, const MoveTreeNode& _node)
	{
		size_t n = 0;
		const auto _op = [&n](const Board& _previousBoard, Move _move)
		{
			if (is_piece_capture(_previousBoard, _move))
			{
				++n;
			};
		};
		foreach_final_move(_board, _node, _op);
		return n;
	};
	inline size_t count_final_castles(const Board& _board, const MoveTreeNode& _node)
	{
		size_t n = 0;
		const auto _op = [&n](const Board& _previousBoard, Move _move)
		{
			if (_previousBoard.get(_move.from()) == Piece::king &&
				chess::distance(_move.from().file(), _move.to().file()) > 1)
			{
				++n;
			};
		};
		foreach_final_move(_board, _node, _op);
		return n;
	};
	inline size_t count_final_checkmates(const Board& _board, const MoveTreeNode& _node)
	{
		size_t n = 0;
		const auto _op = [&n](const Board& _board)
		{
			if (is_checkmate(_board, Color::white) || is_checkmate(_board, Color::black))
			{
				++n;
			};
		};
		foreach_final_position(_board, _node, _op);
		return n;
	};
	inline size_t count_final_double_checks(const Board& _board, const MoveTreeNode& _node)
	{
		size_t n = 0;
		const auto _op = [&n](const Board& _board)
		{
			if (is_double_check(_board, Color::black) || is_double_check(_board, Color::white))
			{
				++n;
			};
		};
		foreach_final_position(_board, _node, _op);
		return n;
	};
	inline size_t count_final_enpassants(const Board& _board, const MoveTreeNode& _node)
	{
		size_t n = 0;
		const auto _op = [&n](const Board& _previousBoard, Move _move)
		{
			if (is_enpassant(_previousBoard, _move))
			{
				++n;
			};
		};
		foreach_final_move(_board, _node, _op);
		return n;
	};

	inline std::vector<std::pair<Board, Move>>
		find_final_check_moves(const Board& _board, const MoveTreeNode& _node)
	{
		auto o = std::vector<std::pair<Board, Move>>();
		const auto _op = [&o](const Board& _board, Move _move)
		{
			auto nb = _board;
			nb.move(_move);

			if (is_check(nb, Color::white) || is_check(nb, Color::black))
			{
				o.push_back({ nb, _move });
			};
		};
		foreach_final_move(_board, _node, _op);
		return o;
	};
	

	namespace impl
	{
		struct PrunedNode
		{
			Board previous_board;
			MoveTreeNode node;
		};
	};

	struct MoveTree
	{
	private:
		void resort_children(std::mt19937* _rnd);

		void evaluate_next_propogate(MoveTreeSearchData _searchData, const MoveTreeProfile& _profile);

	public:
		// Common evaluate next function
		void evaluate_next(MoveTreeSearchData _searchData, const MoveTreeProfile& _profile = MoveTreeProfile());

		std::optional<RatedMove> best_move();
		std::optional<RatedMove> best_move(std::mt19937& _rnd) { return this->best_move(); };

		size_t tree_size() const;
		size_t total_outcomes() const;

		std::vector<std::vector<const MoveTreeNode*>> get_top_lines(size_t _maxCount) const;

		size_t count_unique_positions();
		size_t count_checks();

		void build_tree(size_t _depth, size_t _maxExtendedDepth, const MoveTreeProfile& _profile = MoveTreeProfile());
		void build_tree(size_t _depth, const MoveTreeProfile& _profile = MoveTreeProfile())
		{
			return this->build_tree(_depth, _depth + 2, _profile);
		};

		/**
		 * @brief Gets the root node for this tree. 
		 * 
		 * The root node holds the last move played for the board this tree starts from.
		 * 
		 * @return Root tree node reference.
		*/
		MoveTreeNode& root() noexcept
		{
			return this->root_;
		};

		/**
		 * @brief Gets the root node for this tree.
		 *
		 * The root node holds the last move played for the board this tree starts from.
		 *
		 * @return Root tree node reference.
		*/
		const MoveTreeNode& root() const noexcept
		{
			return this->root_;
		};

		/**
		 * @brief Gets the initial board state.
		 * @return Chess board state.
		*/
		const chess::Board& initial_board() const
		{
			return this->board_;
		};

		/**
		 * @brief Sets the initial board state.
		 * 
		 * This will cause the tree to be cleared and the root move to be changed.
		 * 
		 * @param _board Chess board state.
		*/
		void set_initial_board(const chess::Board& _board)
		{
			// Set the board.
			this->board_ = _board;

			// Determine the initial board rating, this is required for changing the root node.
			const auto _boardRating = chess::quick_rate(_board, _board.get_toplay());

			// Clear out the root node and set to use the last played move.
			this->root_.clear();
			if (_board.get_last_move())
			{
				this->root_.set_move(RatedMove(_board.get_last_move(), _boardRating),
					!_board.get_toplay());
			};

		};


		MoveTree() = default;
		MoveTree(const chess::Board& _board) :
			root_()
		{
			this->set_initial_board(_board);
		};

	private:

		/**
		 * @brief Holds the initial board state.
		*/
		chess::Board board_{};

		/**
		 * @brief The root node for the tree, holds the last move played for the stored initial board.
		*/
		MoveTreeNode root_;
		
		/**
		 * @brief Probably not needed.
		*/
		size_t depth_counter_ = 0;

#ifdef SCREEPFISH_DEBUG_ALPHABETA
		std::vector<impl::PrunedNode> ab_pruned_nodes_{};
#endif

	};




};