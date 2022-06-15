#include "move_tree.hpp"

#include <iostream>

namespace chess
{
	/**
	 * @brief Enables / disables ignoring repeated positions.
	*/
	constexpr inline bool disable_culling_repeated_positions_v = true;



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
		std::ranges::sort(this->responses_,
			[](const MoveTreeNode& lhs, const MoveTreeNode& rhs)
			{
				const auto lr = lhs.player_rating();
				const auto rr = rhs.player_rating();
				if (lr != rr)
				{
					return lhs.player_rating() >
						rhs.player_rating();
				}
				else
				{
					return lhs.quick_rating() > rhs.quick_rating();
				};
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




	inline NodeEvalResult get_interesting_lines(
		const MoveTreeNode& _node,
		const Board& _previousBoard, const Board& _newBoard, Move _move,
		const MoveTreeProfile& _profile, MoveTreeSearchData _data)
	{
		// Profile aliasing
		const auto& _followChecks = _profile.follow_checks_;
		const auto& _followCaptures = _profile.follow_captures_;

		auto _result = NodeEvalResult();

		// Follow captures if set.
		if (_data.can_go_deeper())
		{
			// Follow captures if set.
			if (_followCaptures && is_piece_capture(_previousBoard, _move))
			{
				_result.follow_capture_ = true;
			}
			// Follow checks if set.
			else if (_followChecks && is_check(_newBoard, _newBoard.get_toplay()))
			{
				_result.follow_check_ = true;
			};
		};

		return _result;
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




	NodeEvalResult MoveTreeNode::evaluate_next_with_board(const Board& _board,
		const MoveTreeProfile& _profile, MoveTreeSearchData _data, bool _autoProp)
	{
		// Profile aliasing
		const auto& _followChecks = _profile.follow_checks_;
		const auto& _followCaptures = _profile.follow_captures_;

		// Grab the opponents color and our color.
		const auto _opponentColor = _board.get_toplay();
		const auto _myColor = !_opponentColor;

		auto _evalResult = NodeEvalResult();

		if (!this->was_evaluated() && _data.can_go_deeper())
		{
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
				it->set_move(
					RatedMove{ *p, _rating },
					_opponentColor
				);
				it->depth_ = this->depth_ + 1;

				// Follow lines based on profile.
				_evalResult = get_interesting_lines(*it, _board, _newBoard, _move, _profile, _data);

				// Next
				++it;
			};

			// Ensure we are marked as evaluated
			this->mark_as_evaluated();
		}
		else if(_autoProp)
		{
			// Propogate to children
			for (auto& _child : *this)
			{
				_child.depth_ = this->depth_ + 1;
				_child.evaluate_next(_board, _profile, _data.with_next_depth(), _autoProp);
			};

			//SCREEPFISH_BREAK();
		};

		return _evalResult;
	};

	NodeEvalResult MoveTreeNode::evaluate_next(const Board& _previousBoard,
		const MoveTreeProfile& _profile, MoveTreeSearchData _data, bool _autoProp)
	{
		auto _board = _previousBoard;
		_board.move(this->move_);
		return this->evaluate_next_with_board(_board, _profile, _data, _autoProp);
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
		std::cout << this->move_ << "(" << this->quick_rating() << ") ";
		if (!this->empty())
		{
			this->front().show_best_line();
		};
	};

	std::vector<RatedMove> MoveTreeNode::get_best_line() const
	{
		auto v = std::vector<RatedMove>{ this->move_ };
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
			b.move(_move.move_);
			const auto h = hash(b, b.get_toplay() == Color::black);
			_boards.insert(h);
			_move.count_duplicates(b, _boards);
		};
	};

	size_t MoveTreeNode::count_checks(Board _board) const
	{
		size_t n = 0;
		for (auto& m : *this)
		{
			auto b = _board;
			b.move(m.move_);
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


	inline auto find_not(auto&& _range, auto&& _op)
	{
		return std::ranges::find(_range, [&_op](auto& v)
			{
				return !_op(v);
			});
	};




	// Invokes a function on each leaf in a tree
	inline auto foreach_leaf(auto&& _tree, auto&& _op)
	{
		auto it = std::ranges::begin(_tree);
		const auto _end = std::ranges::end(_tree);
		if (it == _end)
		{
			// Empty, therefore leaf
			_op(*it);
		}
		else
		{
			for (; it != _end; std::ranges::advance(it, 1))
			{
				foreach_leaf(*it, _op);
			};
		};
	};

	




	inline Rating minimax(const Board& _board, MoveTreeNode& _node,
		bool _isMaximizingPlayer)
	{
		if (_node.empty())
		{
			// Leaf, return node rating.
			return _node.player_rating();
		}
		else
		{
			Rating _value = AbsoluteRating::min().raw();
			for (auto& _move : _node)
			{
				// Play the next move.
				auto _newBoard = _board;
				_newBoard.move(_move.move_);
					
				// Find rating for response move.
				const auto _newBoardRating =
					minimax(_newBoard, _move, !_isMaximizingPlayer);
				_value = std::max(_value, _newBoardRating);
			};

			_node.resort_children();

			// Assign the rating to the node
			auto _absValue = AbsoluteRating(_value, !_node.played_by());
			_node.set_rating(_absValue);
			return -_value;
		};
	};

	inline AbsoluteRating minimax(const Board& _board, MoveTreeNode& _node)
	{
		// Calculate the deep rating
		const auto _deepRating = AbsoluteRating(minimax(_board, _node, true), _node.played_by());
		return _deepRating;
	};

	inline void prune(MoveTreeNode& _node)
	{
		if (_node.empty())
		{
			return;
		};
		
		_node.resize(1);
		return;
	};




	inline void alpha_beta_eval(MoveTreeNode& _node, const Board& _board,
		MoveTreeProfile& _profile, MoveTreeSearchData& _searchData)
	{
		const auto _evalResult = _node.evaluate_next_with_board(_board, _profile, _searchData, false);
		if (_evalResult.follow_capture_)
		{
			++_searchData.max_depth_;
			_profile.follow_captures_ = false;
		}
		else if (_evalResult.follow_check_)
		{
			++_searchData.max_depth_;
			_profile.follow_checks_ = false;
		};
	};

	inline Rating alpha_beta(const Board& _board,
		MoveTreeNode& _node, MoveTreeProfile _profile, MoveTreeSearchData _searchData,
		MoveTreeAlphaBeta _alphaBeta, bool _isMaximizingPlayer)
	{
		if (!_searchData.can_go_deeper() /* or node is a terminal node */ )
		{
			return (_isMaximizingPlayer)?
				-_node.quick_rating() :
				_node.quick_rating();
		};

		SCREEPFISH_ASSERT(_board.get_last_move() == _node.move_);
		alpha_beta_eval(_node, _board, _profile, _searchData);
		
		if (_isMaximizingPlayer)
		{
			auto _value = 
				-Rating(std::numeric_limits<Rating>::infinity());

			for (auto& _move : _node)
			{
				auto _newBoard = _board;
				_newBoard.move(_move.move_);

				_value = std::max
				(
					_value,
					alpha_beta(_newBoard, _move, _profile,
						_searchData.with_next_depth(),
						_alphaBeta, false
					)
				);

				if (_value >= _alphaBeta.beta)
				{
					break; // (*β cutoff*)
				};

				_alphaBeta.alpha = std::max(
					_alphaBeta.alpha,
					_value
				);
			};

			return _value;
		}
		else
		{
			auto _value =
				Rating(std::numeric_limits<Rating>::infinity());

			for (auto& _move : _node)
			{
				auto _newBoard = _board;
				_newBoard.move(_move.move_);

				_value = std::min
				(
					_value,
					alpha_beta(_newBoard, _move, _profile,
						_searchData.with_next_depth(),
						_alphaBeta, true
					)
				);

				if (_value <= _alphaBeta.alpha)
				{
					break; // (*α cutoff*)
				};

				_alphaBeta.beta = std::min(
					_alphaBeta.beta,
					_value
				);
			};

			return _value;
		};

		::abort();
	};










	void MoveTree::evaluate_next(MoveTreeSearchData _searchData, const MoveTreeProfile& _profile)
	{
		auto& _board = this->board_;
		auto& _root = this->root_;

		// Forward to root node.
		_root.evaluate_next_with_board(_board, _profile, _searchData);

		// Probably not needed tbh
		++this->depth_counter_;
	};








	std::optional<RatedMove> MoveTree::best_move()
	{
		auto& _root = this->root();
		if (_root.empty())
		{
			return std::nullopt;
		}
		else
		{
			// Resort to put best in the front.
			this->resort_children(nullptr);

			auto& _best = _root.front();
			auto o = _best.move_;
			o = RatedMove(o.from(), o.to(), _best.player_rating());
			return o;
		};
	};

	size_t MoveTree::tree_size() const
	{
		auto& _moves = this->root();

		size_t n = _moves.size();
		for (auto& v : _moves)
		{
			n += v.tree_size();
		};
		return n;
	};

	size_t MoveTree::total_outcomes() const
	{
		auto& _moves = this->root();

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
		auto& _moves = this->root();

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
		auto& _moves = this->root();
		auto& _board = this->board_;

		auto bs = std::set<size_t>();
		for (auto& _move : _moves)
		{
			auto b = _board;
			b.move(_move.move_);
			bs.insert(hash(b, b.get_toplay() == Color::black));
			_move.count_duplicates(b, bs);
		};
		return bs.size();
	};
	size_t MoveTree::count_checks()
	{
		auto& _moves = this->root();
		auto& _board = this->board_;
		return _moves.count_checks(_board);
	};
	



	void MoveTree::resort_children(std::mt19937* _rnd)
	{
		auto& _root = this->root();
		_root.resort_children();
	};
	
	void MoveTree::build_tree(size_t _depth, size_t _maxExtendedDepth, const MoveTreeProfile& _profile)
	{
		// Reset state
		auto& _root = this->root();
		_root.clear();

		// Additional move tree searching data storage
		auto _searchData = MoveTreeSearchData();
		_searchData.max_depth_ = static_cast<uint8_t>(_maxExtendedDepth);

		if (_profile.alphabeta_)
		{
			auto _alphaBeta = MoveTreeAlphaBeta();
			_alphaBeta.alpha = -std::numeric_limits<Rating>::infinity();
			_alphaBeta.beta = std::numeric_limits<Rating>::infinity();

			const auto _alphaBetaRating = alpha_beta(
				this->initial_board(), _root,
				_profile, _searchData, _alphaBeta, true);
		}
		else
		{
			for (size_t n = 0; n != _depth; ++n)
			{
				this->root().evaluate_next(this->initial_board(), _profile, _searchData,
					true);
			};
		};

		minimax(this->initial_board(), _root);
		this->resort_children(nullptr);

		//_root.clear();
		//
		//for (size_t n = 0; n != _depth; ++n)
		//{
		//	this->evaluate_next(_searchData, _profile);
		//};
		//
		//const auto _count = this->root().tree_size();
		//deep_eval(_root);
		//this->resort_children(nullptr);
		//std::cout << "Tree Search Eval : " << this->root().front().player_rating() << '\n';
		//SCREEPFISH_BREAK();
	};





};
