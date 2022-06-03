#include "move_tree.hpp"

#include <iostream>

namespace chess
{
	void MoveTreeNode::evaluate_next(const Board& _previousBoard, bool _followChecks)
	{
		// Apply our move.
		auto _board = _previousBoard;
		const auto _myColor = _board.get_toplay();
		_board.move(this->move);

		if (this->empty())
		{
			// Get the possible responses
			std::array<Move, 128> _moveBufferData{};
			auto _moveBuffer = MoveBuffer(_moveBufferData.data(), _moveBufferData.data() + _moveBufferData.size());
			const auto _moveBegin = _moveBuffer.head();
			const auto _opponentColor = _board.get_toplay();
			get_moves(_board, _opponentColor, _moveBuffer);
			const auto _moveEnd = _moveBuffer.head();

			// Rate and add to the child nodes
			this->resize(_moveEnd - _moveBegin);
			auto it = this->begin();
			for (auto p = _moveBegin; p != _moveEnd; ++p)
			{
				// Rate the move
				auto _newBoard = _board;
				_newBoard.move(*p);
				const auto _rating = rate_board(_newBoard, _opponentColor);

				// Assign values
				it->move = RatedMove{ *p, _rating };
				it->rating_ = _rating;

				//it->hash = _hash;

				if (_followChecks && is_check(_newBoard, _myColor))
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
			for (auto& _child : *this)
			{
				_child.evaluate_next(_board, _followChecks);
			};
		};

		// Sort children by rating
		std::sort(this->begin(), this->end(), [](const MoveTreeNode& lhs, const MoveTreeNode& rhs) -> bool
			{
				return lhs.rating() > rhs.rating();
			});

		// Set our rating to show the best opponent response
		if (!this->empty())
		{
			this->rating_ = -this->front().rating();
		};
	};

	size_t MoveTreeNode::tree_size() const
	{
		size_t n = this->size();
		for (auto& v : *this)
		{
			n += v.tree_size();
		};
		return n;
	};
	size_t MoveTreeNode::total_outcomes() const
	{
		size_t n = 0;
		for (auto& v : *this)
		{
			if (v.empty())
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
		std::cout << this->move << "(" << this->quick_rating() << ") ";
		if (!this->empty())
		{
			this->front().show_best_line();
		};
	};

	std::vector<RatedMove> MoveTreeNode::get_best_line() const
	{
		auto v = std::vector<RatedMove>{ this->move };
		if (!this->empty())
		{
			auto rv = this->front().get_best_line();
			v.insert(v.end(), rv.begin(), rv.end());
		};
		return v;
	};



	// MoveTree
	//  -> black moves ...
	//		-> white moves ...
	//			-> black moves ...
	//				-> white moves ...






	void MoveTree::evalulate_next()
	{
		auto& _board = this->board;
		if (this->moves.empty())
		{
			// Get the possible responses
			std::array<Move, 128> _moveBufferData{};
			auto _moveBuffer = MoveBuffer(_moveBufferData.data(), _moveBufferData.data() + _moveBufferData.size());
			const auto _moveBegin = _moveBuffer.head();
			get_moves(_board, _board.get_toplay(), _moveBuffer);
			const auto _moveEnd = _moveBuffer.head();

			// Rate and add to the child nodes
			this->moves.resize(_moveEnd - _moveBegin);
			auto it = this->moves.begin();
			for (auto p = _moveBegin; p != _moveEnd; ++p)
			{
				// Rate the move
				auto _newBoard = _board;
				_newBoard.move(*p);
			
				const auto _rating = rate_board(_newBoard, _board.get_toplay());

				// Assign values
				it->move = RatedMove{ *p, _rating };
				it->rating_ = _rating;

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
				return lhs.rating() > rhs.rating();
			});

		++this->depth_counter_;
	};

	std::optional<Move> MoveTree::best_move(std::mt19937& _rnd)
	{
		if (this->moves.empty())
		{
			return std::nullopt;
		}
		else
		{
			for (auto& v : this->moves)
			{
				if (v.empty())
				{
					return v.move;
				};
			};

			auto& _best = this->moves.front();
			const auto it = std::ranges::find_if(this->moves, [&_best](MoveTreeNode& v)
				{
					return v.rating() != _best.rating();
				});

		 	const auto _rndNum = _rnd();
			const auto _rndIndex = _rndNum % (it - this->moves.begin());
			const auto _rndIter = this->moves.begin() + _rndIndex;
			_rndIter->show_best_line();
			std::cout << std::endl;

			return _rndIter->move;
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

	std::vector<std::vector<RatedMove>> MoveTree::get_top_lines(size_t _maxCount) const
	{
		auto o = std::vector<std::vector<RatedMove>>{};
		for (size_t n = 0; n != _maxCount; ++n)
		{
			if (n >= this->moves.size()) { break; };
			o.push_back(this->moves.at(n).get_best_line());
		};
		return o;
	};

};
