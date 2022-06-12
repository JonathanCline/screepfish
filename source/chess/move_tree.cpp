#include "move_tree.hpp"

#include <iostream>

namespace chess
{
	void MoveTreeNode::resort_children()
	{
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
				const auto _rating = quick_rate(_newBoard, _opponentColor);

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

		// Resort children?
		this->resort_children();
	};
	void MoveTreeNode::evaluate_next(const Board& _previousBoard, BoardHashSet& _hashSet, bool _followChecks)
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
				
				// Ensure this is a unique position.
				const auto h = hash(_newBoard, _newBoard.get_toplay() == Color::black);
				if (_hashSet.contains(h))
				{
					// Skip, position is already being evaluated
					continue;
				}
				else
				{
					_hashSet.insert(h);
				};
				
				const auto _rating = quick_rate(_newBoard, _opponentColor);

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

			// Set size to "remove" extra
			this->responses_count_ = it - this->begin();
		}
		else
		{
			// Propogate to children
			for (auto& _child : *this)
			{
				_child.evaluate_next(_board, _hashSet, _followChecks);
			};
		};

		// Resort children?
		this->resort_children();
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

	void MoveTreeNode::count_duplicates(Board _board, std::set<size_t>& _boards)
	{
		// Apply our move.
		for (auto it = this->begin(); it != this->end(); ++it)
		{
			auto& _move = *it;
			auto b = _board;
			b.move(_move.move);
			const auto h = hash(b, b.get_toplay() == Color::black);
			_boards.insert(h);
			_move.count_duplicates(b, _boards);
		};
	};

	size_t MoveTreeNode::count_checks(Board _board)
	{
		size_t n = 0;
		for (auto& m : *this)
		{
			auto b = _board;
			b.move(m.move);
			if (is_check(b, Color::white) || is_check(b, Color::black))
			{
				++n;
			};
			n += m.count_checks(b);
		};
		return n;
	};



	// MoveTree
	//  -> black moves ...
	//		-> white moves ...
	//			-> black moves ...
	//				-> white moves ...





	void MoveTree::evaluate_next_unique()
	{
		auto& _hashSet = this->hash_set_;
		auto& _board = this->board_;
		auto& _moves = this->moves_;

		if (_moves.empty())
		{
			// Get the possible responses
			std::array<Move, 128> _moveBufferData{};
			auto _moveBuffer = MoveBuffer(_moveBufferData.data(), _moveBufferData.data() + _moveBufferData.size());
			const auto _moveBegin = _moveBuffer.head();
			get_moves(_board, _board.get_toplay(), _moveBuffer);
			const auto _moveEnd = _moveBuffer.head();

			// Rate and add to the child nodes
			_moves.resize(_moveEnd - _moveBegin);
			auto it = _moves.begin();
			for (auto p = _moveBegin; p != _moveEnd; ++p)
			{
				// Rate the move
				auto _newBoard = _board;
				_newBoard.move(*p);

				// Ensure this is registered as unique position
				const auto h = hash(_newBoard, _newBoard.get_toplay() == Color::black);
				_hashSet.insert(h);

				const auto _rating = quick_rate(_newBoard, _board.get_toplay());

				// Assign values
				it->move = RatedMove{ *p, _rating };
				it->set_rating(_rating);

				// Next
				++it;
			};
		}
		else
		{
			// Propogate to children
			for (auto& _child : _moves)
			{
				_child.evaluate_next(_board, _hashSet, false);
			};
		};

		// Sort children by rating
		std::ranges::sort(_moves, [](const MoveTreeNode& lhs, const MoveTreeNode& rhs) -> bool
			{
				return lhs.rating() > rhs.rating();
			});

		++this->depth_counter_;
	};
	void MoveTree::evaluate_next()
	{
		auto& _board = this->board_;
		auto& _moves = this->moves_;

		if (_moves.empty())
		{
			// Get the possible responses
			std::array<Move, 128> _moveBufferData{};
			auto _moveBuffer = MoveBuffer(_moveBufferData.data(), _moveBufferData.data() + _moveBufferData.size());
			const auto _moveBegin = _moveBuffer.head();
			get_moves(_board, _board.get_toplay(), _moveBuffer);
			const auto _moveEnd = _moveBuffer.head();

			// Rate and add to the child nodes
			_moves.resize(_moveEnd - _moveBegin);
			auto it = _moves.begin();
			for (auto p = _moveBegin; p != _moveEnd; ++p)
			{
				// Rate the move
				auto _newBoard = _board;
				_newBoard.move(*p);
				const auto _rating = quick_rate(_newBoard, _board.get_toplay());

				// Assign values
				it->move = RatedMove{ *p, _rating };
				it->set_rating(_rating);

				// Next
				++it;
			};
		}
		else
		{
			// Propogate to children
			for (auto& _child : _moves)
			{
				_child.evaluate_next(_board, false);
			};
		};

		// Sort children by rating
		std::ranges::sort(_moves, [](const MoveTreeNode& lhs, const MoveTreeNode& rhs) -> bool
			{
				return lhs.rating() > rhs.rating();
			});

		++this->depth_counter_;
	};

	void MoveTree::evalulate_next()
	{
		return this->evaluate_next();
	};



	std::optional<RatedMove> MoveTree::best_move(std::mt19937& _rnd)
	{
		auto& _moves = this->moves_;

		if (_moves.empty())
		{
			return std::nullopt;
		}
		else
		{
			for (auto& v : _moves)
			{
				if (v.empty())
				{
					return v.move;
				};
			};

			auto& _best = _moves.front();
			const auto it = std::ranges::find_if(_moves, [&_best](MoveTreeNode& v)
				{
					return v.rating() != _best.rating();
				});

		 	const auto _rndNum = _rnd();
			const auto _rndIndex = _rndNum % (it - _moves.begin());
			const auto _rndIter = _moves.begin() + _rndIndex;
			_rndIter->show_best_line();
			std::cout << std::endl;

			auto o = _rndIter->move;
			o = RatedMove(o.from(), o.to(), _rndIter->rating());
			return o;
		};
	};

	size_t MoveTree::tree_size() const
	{
		auto& _moves = this->moves_;

		size_t n = _moves.size();
		for (auto& v : _moves)
		{
			n += v.tree_size();
		};
		return n;
	};

	size_t MoveTree::total_outcomes() const
	{
		auto& _moves = this->moves_;

		size_t n = 0;
		for (auto& v : _moves)
		{
			n += v.total_outcomes();
		};

		if (n == 0)
		{
			n = _moves.size();
		};
		return n;
	};

	std::vector<std::vector<RatedMove>> MoveTree::get_top_lines(size_t _maxCount) const
	{
		auto& _moves = this->moves_;

		auto o = std::vector<std::vector<RatedMove>>{};
		for (size_t n = 0; n != _maxCount; ++n)
		{
			if (n >= _moves.size())
			{
				break;
			};
			o.push_back(_moves.at(n).get_best_line());
		};
		return o;
	};


	size_t MoveTree::count_unique_positions()
	{
		auto& _moves = this->moves_;
		auto& _board = this->board_;

		auto bs = std::set<size_t>();
		for (auto& _move : _moves)
		{
			auto b = _board;
			b.move(_move.move);
			bs.insert(hash(b, b.get_toplay() == Color::black));
			_move.count_duplicates(b, bs);
		};
		return bs.size();
	};
	
	size_t MoveTree::count_checks()
	{
		auto& _moves = this->moves_;
		auto& _board = this->board_;

		size_t n = 0;
		for (auto& m : _moves)
		{
			auto b = _board;
			b.move(m.move);
			if (is_check(b, Color::white) || is_check(b, Color::black))
			{
				++n;
			};
			n += m.count_checks(b);
		};
		return n;
	};
	
	void MoveTree::build_tree(size_t _depth)
	{
		auto& _moves = this->moves_;

		// Reset state
		this->clear_hashes();
		_moves.clear();

		if (_depth <= 2)
		{
			// Do not worry about repeated positions - there wont be any
			for (size_t n = 0; n != _depth; ++n)
			{
				this->evaluate_next();
			};
		}
		else
		{
			// Watch for repeated positions - this is where we actually benefit from checking
			for (size_t n = 0; n != _depth; ++n)
			{
				this->evaluate_next_unique();
			};
		};
	};

};
