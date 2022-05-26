#include "move_tree.hpp"

#include <iostream>

namespace chess
{
	void MoveTreeNode::evaluate_next(const Board& _previousBoard)
	{
		// Apply our move.
		auto _board = _previousBoard;
		_board.move(this->move.move);

		if (this->responses.empty())
		{
			// Get the possible responses
			std::array<Move, 128> _moveBufferData{};
			auto _moveBuffer = MoveBuffer(_moveBufferData.data(), _moveBufferData.data() + _moveBufferData.size());
			const auto _moveBegin = _moveBuffer.head();
			get_moves(_board, !this->move_played_by, _moveBuffer);
			const auto _moveEnd = _moveBuffer.head();

			// Rate and add to the child nodes
			this->responses.resize(_moveEnd - _moveBegin);
			auto it = this->responses.begin();
			for (auto p = _moveBegin; p != _moveEnd; ++p)
			{
				// Rate the move
				auto _newBoard = _board;
				_newBoard.move(*p);
				const auto _hash = chess::hash(_newBoard, _newBoard.get_toplay() == Color::black);

				const auto _rating = rate_move(_newBoard, *p, !this->move_played_by);

				// Assign values
				it->move_played_by = !this->move_played_by;
				it->move = RatedMove{ *p, _rating };
				it->hash = _hash;

				// Next
				++it;
			};

		}
		else
		{
			// Propogate to children
			for (auto& _child : this->responses)
			{
				_child.evaluate_next(_board);
			};
		};

		// Sort children by rating
		std::ranges::sort(this->responses, [](const MoveTreeNode& lhs, const MoveTreeNode& rhs) -> bool
			{
				return lhs.move.rating > rhs.move.rating;
			});

		// Set our rating to show the best opponent response
		if (!this->responses.empty())
		{
			this->move.rating = -this->responses.front().move.rating;
		};
	};

	size_t MoveTreeNode::tree_size() const
	{
		size_t n = this->responses.size();
		for (auto& v : this->responses)
		{
			n += v.tree_size();
		};
		return n;
	};
	size_t MoveTreeNode::total_outcomes() const
	{
		size_t n = 0;
		for (auto& v : this->responses)
		{
			if (v.responses.empty())
			{
				n += 1;
			}
			else
			{
				n += v.total_outcomes();
			};
		};
		return n;
	};



	// MoveTree
	//  -> black moves ...
	//		-> white moves ...
	//			-> black moves ...
	//				-> white moves ...






	void MoveTree::evalulate_next()
	{
		// Apply our move.
		auto& _board = this->board;

		if (this->moves.empty())
		{
			// Get the possible responses
			std::array<Move, 128> _moveBufferData{};
			auto _moveBuffer = MoveBuffer(_moveBufferData.data(), _moveBufferData.data() + _moveBufferData.size());
			const auto _moveBegin = _moveBuffer.head();
			get_moves(_board, this->to_play, _moveBuffer);
			const auto _moveEnd = _moveBuffer.head();

			// Rate and add to the child nodes
			this->moves.resize(_moveEnd - _moveBegin);
			auto it = this->moves.begin();
			for (auto p = _moveBegin; p != _moveEnd; ++p)
			{
				// Rate the move
				auto _newBoard = _board;
				_newBoard.move(*p);
				const auto _hash = hash(_newBoard, _newBoard.get_toplay() == Color::black);
			
				const auto _rating = rate_move(_newBoard, *p, this->to_play);

				// Assign values
				it->move_played_by = this->to_play;
				it->move = RatedMove{ *p, _rating };
				it->hash = _hash;

				// Next
				++it;
			};
		}
		else
		{
			// Propogate to children
			for (auto& _child : this->moves)
			{
				_child.evaluate_next(_board);
			};
		};

		// Sort children by rating
		std::ranges::sort(this->moves, [](const MoveTreeNode& lhs, const MoveTreeNode& rhs) -> bool
			{
				return lhs.move.rating > rhs.move.rating;
			});
	};

	std::optional<Move> MoveTree::best_move()
	{
		MoveTreeNode* at = this->moves.data();
		while (at)
		{
			std::cout << at->move.move << ", ";
			if (at->responses.empty())
				break;
			at = at->responses.data();
		};
		std::cout << '\n';

		if (this->moves.empty())
		{
			return std::nullopt;
		}
		else
		{
			return this->moves.front().move.move;
		};
	};

	size_t MoveTree::tree_size() const
	{
		size_t n = this->moves.size();
		for (auto& v : this->moves)
		{
			n += v.tree_size();
		};
		return n;
	};

	size_t MoveTree::total_outcomes() const
	{
		size_t n = 0;
		for (auto& v : this->moves)
		{
			n += v.total_outcomes();
		};
		if (n == 0)
		{
			n = this->moves.size();
		};
		return n;
	};

};
