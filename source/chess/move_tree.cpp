#include "move_tree.hpp"

#include <iostream>

namespace chess
{
	void MoveTreeNode::evaluate_next(const Board& _previousBoard, bool _followChecks)
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
				//const auto _hash = chess::hash(_newBoard, _newBoard.get_toplay() == Color::black);

				const auto _rating = rate_board(_newBoard, !this->move_played_by);

				// Assign values
				it->move_played_by = !this->move_played_by;
				it->move = RatedMove{ *p, _rating };
				//it->hash = _hash;

				if (_followChecks && is_check(_newBoard, it->move_played_by))
				{
					it->evaluate_next(_board, false);
				};

				// Next
				++it;
			};

		}
		else
		{
			// Propogate to children
			for (auto& _child : this->responses)
			{
				_child.evaluate_next(_board, _followChecks);
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

	void MoveTreeNode::show_best_line() const
	{
		std::cout << this->move.move << ' ';
		if (!this->responses.empty())
		{
			this->responses.front().show_best_line();
		};
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
			
				const auto _rating = rate_board(_newBoard, this->to_play);

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
				_child.evaluate_next(_board, false);
			};
		};

		// Sort children by rating
		std::ranges::sort(this->moves, [](const MoveTreeNode& lhs, const MoveTreeNode& rhs) -> bool
			{
				return lhs.move.rating > rhs.move.rating;
			});
	};

	std::optional<Move> MoveTree::best_move(std::mt19937& _rnd)
	{
		if (this->moves.empty())
		{
			return std::nullopt;
		}
		else
		{
			auto& _best = this->moves.front().move;
			const auto it = std::ranges::find_if(this->moves, [&_best](MoveTreeNode& v)
				{
					return v.move.rating != _best.rating;
				});

		 	const auto _rndNum = _rnd();
			const auto _rndIndex = _rndNum % (it - this->moves.begin());
			const auto _rndIter = this->moves.begin() + _rndIndex;
			std::cout << _rndIter->move.move << ' ';
			_rndIter->show_best_line();
			std::cout << std::endl;

			return _rndIter->move.move;
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
