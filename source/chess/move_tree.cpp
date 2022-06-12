#include "move_tree.hpp"

#include <iostream>

namespace chess
{
	/**
	 * @brief Enables / disables ignoring repeated positions.
	*/
	constexpr inline bool disable_culling_repeated_positions_v = false;



	namespace impl
	{
		MoveTreeNodeBlockAllocator::pointer MoveTreeNodeBlockAllocator::allocate(size_t n) const
		{
			auto _mem = new value_type[n]{};
			SCREEPFISH_ASSERT(_mem);
			return _mem;
		};
		void MoveTreeNodeBlockAllocator::deallocate(pointer p) const
		{
			SCREEPFISH_ASSERT(p);
			delete[] p;
		};
	};


	void MoveTreeNode::resort_children()
	{
		// Sort children by rating
		std::sort(this->begin(), this->end(), [](const MoveTreeNode& lhs, const MoveTreeNode& rhs) -> bool
			{
				return lhs.rating() > rhs.rating();
			});
	};


	template <size_t Max = 128>
	inline auto fill_possible_moves_buffer(const Board& _board, std::array<Move, Max>& _bufferData)
	{
		// Get the possible responses
		auto _moveBuffer = MoveBuffer
		(
			_bufferData.data(),
			_bufferData.data() + _bufferData.size()
		);
		
		const auto _moveBegin = _moveBuffer.head();
		const auto _opponentColor = _board.get_toplay();
		get_moves(_board, _opponentColor, _moveBuffer);
		return _moveBuffer.head();
	};




	inline void follow_move_lines_with_profile(
		MoveTreeNode& _node,
		const Board& _previousBoard, const Board& _newBoard, Move _move,
		const MoveTreeProfile& _profile, MoveTreeSearchData _data)
	{
		// Profile aliasing
		const auto& _followChecks = _profile.follow_checks_;
		const auto& _followCaptures = _profile.follow_captures_;

		// Follow captures if set.
		if (_data.can_go_deeper())
		{
			// Follow captures if set.
			if (_followCaptures && is_piece_capture(_previousBoard, _move))
			{
				auto _nestedProf = _profile;
				_nestedProf.follow_captures_ = false;
				_node.evaluate_next(_previousBoard, _nestedProf, _data.with_next_depth());
			}
			// Follow checks if set.
			else if (_followChecks && is_check(_newBoard, _newBoard.get_toplay()))
			{
				_node.evaluate_next(_previousBoard, _profile, _data.with_next_depth());
			};
		};
	};


	void MoveTreeNode::evaluate_next(const Board& _previousBoard,
		const MoveTreeProfile& _profile, MoveTreeSearchData _data)
	{
		// Profile aliasing
		const auto& _followChecks = _profile.follow_checks_;
		const auto& _followCaptures = _profile.follow_captures_;


		// Apply our move.
		auto _board = _previousBoard;
		const auto _myColor = _board.get_toplay();
		_board.move(this->move);

		const auto _opponentColor = !_myColor;

		if (!this->was_evaluated())
		{
			// Bail early on max depth
			if (!_data.can_go_deeper())
			{
				this->mark_as_evaluated();
				return;
			};

			// Get the possible responses
			std::array<Move, 128> _moveBufferData{};
			const auto _moveBegin = _moveBufferData.data();
			const auto _moveEnd = fill_possible_moves_buffer(_board, _moveBufferData);

			// Rate and add to the child nodes
			this->resize(_moveEnd - _moveBegin);
			auto it = this->begin();
			for (auto p = _moveBegin; p != _moveEnd; ++p)
			{
				// Apply the move to the board with our move played.
				const auto& _move = *p;
				auto _newBoard = _board;
				_newBoard.move(_move);

				// Lightning fast rating.
				const auto _rating = quick_rate(_newBoard, _opponentColor);

				// Assign values
				it->move = RatedMove{ *p, _rating };
				it->rating_ = _rating;

				// Follow lines based on profile.
				follow_move_lines_with_profile(*it, _board, _newBoard, _move, _profile, _data);

				// Next
				++it;
			};

			// Ensure we are marked as evaluated
			this->mark_as_evaluated();
		}
		else
		{
			// Propogate to children
			for (auto& _child : *this)
			{
				_child.evaluate_next(_board, _profile, _data.with_next_depth());
			};
		};
	};
	void MoveTreeNode::evaluate_next(const Board& _previousBoard, BoardHashSet& _hashSet,
		const MoveTreeProfile& _profile, MoveTreeSearchData _data)
	{
		// Profile aliasing
		const auto& _followChecks = _profile.follow_checks_;
		const auto& _followCaptures = _profile.follow_captures_;

		// Apply our move.
		const auto& _myMove = this->move;
		auto _board = _previousBoard;
		const auto _myColor = _board.get_toplay();
		_board.move(_myMove);

		const auto _opponentColor = !_myColor;

		if (!this->was_evaluated())
		{
			// Bail early on max depth
			if (!_data.can_go_deeper())
			{
				this->mark_as_evaluated();
				return;
			};

			// Get the possible responses
			std::array<Move, 128> _moveBufferData{};
			const auto _moveBegin = _moveBufferData.data();
			const auto _moveEnd = fill_possible_moves_buffer(_board, _moveBufferData);

			// Rate and add to the child nodes
			this->resize(_moveEnd - _moveBegin);
			auto it = this->begin();
			for (auto p = _moveBegin; p != _moveEnd; ++p)
			{
				// Apply the move to the board with our move played.
				const auto& _move = *p;
				auto _newBoard = _board;
				_newBoard.move(_move);

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

				// Lightning fast rating.
				const auto _rating = quick_rate(_newBoard, _opponentColor);

				// Assign values
				it->move = RatedMove{ *p, _rating };
				it->rating_ = _rating;

				// Follow lines based on profile.
				follow_move_lines_with_profile(*it, _board, _newBoard, _move, _profile, _data);

				// Next
				++it;
			};

			// Set size to "remove" extra
			this->soft_resize(it - this->begin());

			// Ensure we are marked as evaluated
			this->mark_as_evaluated();
		}
		else
		{
			// Propogate to children
			for (auto& _child : *this)
			{
				_child.evaluate_next(_board, _hashSet, _profile, _data.with_next_depth());
			};
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



	void MoveTree::evaluate_next(MoveTreeSearchData _searchData, BoardHashSet* _hashSet, const MoveTreeProfile& _profile)
	{
		auto& _board = this->board_;
		auto& _moves = this->moves_;

		if (_moves.empty())
		{
			// Get the possible responses
			std::array<Move, 128> _moveBufferData{};
			const auto _moveBegin = _moveBufferData.data();
			const auto _moveEnd = fill_possible_moves_buffer(_board, _moveBufferData);

			// Rate and add to the child nodes
			_moves.resize(_moveEnd - _moveBegin);
			auto it = _moves.begin();
			for (auto p = _moveBegin; p != _moveEnd; ++p)
			{
				// Rate the move
				auto _newBoard = _board;
				_newBoard.move(*p);

				// Ensure this is registered as unique position
				if (_hashSet)
				{
					const auto h = hash(_newBoard, _newBoard.get_toplay() == Color::black);
					_hashSet->insert(h);
				};

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
				if (_hashSet)
				{
					_child.evaluate_next(_board, *_hashSet, _profile, _searchData.with_next_depth());
				}
				else
				{
					_child.evaluate_next(_board, _profile, _searchData.with_next_depth());
				};
			};
		};

		++this->depth_counter_;
	};

	void MoveTree::evaluate_next_unique(MoveTreeSearchData _searchData, const MoveTreeProfile& _profile)
	{
		auto& _hashSet = this->hash_set_;
		return this->evaluate_next(_searchData, &_hashSet, _profile);
	};
	void MoveTree::evaluate_next(MoveTreeSearchData _searchData, const MoveTreeProfile& _profile)
	{
		BoardHashSet* _hashSet = nullptr;
		return this->evaluate_next(_searchData, _hashSet, _profile);
	};

	void MoveTree::evalulate_next(MoveTreeSearchData _searchData, const MoveTreeProfile& _profile)
	{
		return this->evaluate_next(_searchData, _profile);
	};





	inline Rating deep_eval(MoveTreeNode& _node)
	{
		// If the opponent has no responses, we can just set the rating as the quick rating
		// and return early. These "ends" of the tree are the basis for the deep evaluation.
		if (_node.empty())
		{
			// Use the quick rating.
			const auto _rating = _node.quick_rating();
			_node.set_rating(_rating);
			return _rating;
		};

		// Loop through the opponent's responses to the node's move and preform a deep eval for each.
		bool _hasMoveAfterOpponent = false;
		for (auto& _opponentResponse : _node)
		{
			// Preform a deep evaluation of the opponent's response.
			const auto _opponentResponseRating = deep_eval(_opponentResponse);
			if (!_opponentResponse.empty())
			{
				_hasMoveAfterOpponent = true;
			};
		};

		// Sort the opponent's responses by full rating.
		_node.resort_children();

		const auto _rating = -_node.front().rating();
		_node.set_rating(_rating);
		return _rating;
	};








	std::optional<RatedMove> MoveTree::best_move(std::mt19937& _rnd)
	{
		auto& _moves = this->moves_;

		// Preform a deep evaluation.
		for (auto& _move : _moves)
		{
			deep_eval(_move);
		};


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

			std::ranges::sort(this->moves_, [](auto& lhs, auto& rhs)
				{
					return lhs.rating() > rhs.rating();
				});

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
	
	void MoveTree::build_tree(size_t _depth, size_t _maxExtendedDepth, const MoveTreeProfile& _profile)
	{
		// Reset state
		this->clear_hashes();
		auto& _moves = this->moves_;
		_moves.clear();

		// Additional move tree searching data storage
		auto _searchData = MoveTreeSearchData();
		_searchData.max_depth_ = static_cast<uint8_t>(_maxExtendedDepth);

		// Determine whether or not to cull unique
		if (disable_culling_repeated_positions_v || (_depth <= 3))
		{
			// Do not worry about repeated positions - there wont be any
			for (size_t n = 0; n != _depth; ++n)
			{
				this->evaluate_next(_searchData, _profile);
			};
		}
		else
		{
			// Watch for repeated positions - this is where we actually benefit from checking
			for (size_t n = 0; n != _depth; ++n)
			{
				this->evaluate_next_unique(_searchData, _profile);
			};
		};
	};

};
