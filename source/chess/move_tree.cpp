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
				return lhs.player_rating() > rhs.player_rating();
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
		const MoveTreeProfile& _profile, MoveTreeSearchData _data)
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

				// Follow lines based on profile.
				_evalResult = get_interesting_lines(*it, _board, _newBoard, _move, _profile, _data);

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

			//SCREEPFISH_BREAK();
		};

		return _evalResult;
	};

	NodeEvalResult MoveTreeNode::evaluate_next(const Board& _previousBoard,
		const MoveTreeProfile& _profile, MoveTreeSearchData _data)
	{
		auto _board = _previousBoard;
		_board.move(this->move_);
		return this->evaluate_next_with_board(_board, _profile, _data);
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

	size_t MoveTreeNode::count_checks(Board _board)
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





	inline AbsoluteRating deep_eval(MoveTreeNode& _node)
	{
		// If the opponent has no responses, we can just set the rating as the quick rating
		// and return early. These "ends" of the tree are the basis for the deep evaluation.
		if (_node.empty())
		{
			// Use the quick rating.
			return _node.rating();
		};

		// Loop through the opponent's responses to the node's move and preform a deep eval for each.
		bool _hasMoveAfterOpponent = false;
		for (auto& _opponentResponse : _node)
		{
			// Preform a deep evaluation of the opponent's response.
			const auto _opponentResponseRating =
				deep_eval(_opponentResponse);

			if (!_opponentResponse.empty())
			{
				_hasMoveAfterOpponent = true;
			};
		};

		// Sort the opponent's responses by full rating.
		_node.resort_children();

		// The opponent's best response will be in the front.
		const auto _bestResponseRating = _node.front().rating();
		
		_node.set_rating(_bestResponseRating);
		
		return _bestResponseRating;
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




	Rating alpha_beta_max(const Board& _previousBoard,
		MoveTreeNode& _node, MoveTreeProfile _profile,
		MoveTreeSearchData _searchData, MoveTreeAlphaBeta _alphaBeta);

	Rating alpha_beta_min(const Board& _previousBoard,
		MoveTreeNode& _node, MoveTreeProfile _profile,
		MoveTreeSearchData _searchData, MoveTreeAlphaBeta _alphaBeta);



	inline void alpha_beta_eval(MoveTreeNode& _node, const Board& _board,
		MoveTreeProfile& _profile, MoveTreeSearchData& _searchData)
	{
		const auto _evalResult = _node.evaluate_next_with_board(_board, _profile, _searchData);
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


	Rating alpha_beta_max(const Board& _board,
		MoveTreeNode& _node, MoveTreeProfile _profile,
		MoveTreeSearchData _searchData, MoveTreeAlphaBeta _alphaBeta)
	{
		if (!_searchData.can_go_deeper())
		{
			return _node.quick_rating();
		};

		alpha_beta_eval(_node, _board, _profile, _searchData);

		size_t n = 0;
		for (auto& _move : _node)
		{
			auto _newBoard = _board;
			_newBoard.move(_move.move_);

			auto _score = alpha_beta_min(_newBoard, _move, _profile,
				_searchData.with_next_depth(), _alphaBeta);

			if (_score >= _alphaBeta.beta)
			{
				return _alphaBeta.beta;   // fail hard beta-cutoff
			}
			if (_score > _alphaBeta.alpha)
			{
				_alphaBeta.alpha = _score; // alpha acts like max in MiniMax
			};

			++n;
		};

		return _alphaBeta.alpha;
	};

	Rating alpha_beta_min(const Board& _board,
		MoveTreeNode& _node, MoveTreeProfile _profile,
		MoveTreeSearchData _searchData, MoveTreeAlphaBeta _alphaBeta)
	{
		if (!_searchData.can_go_deeper())
		{
			return -_node.quick_rating();
		};

		alpha_beta_eval(_node, _board, _profile, _searchData);

		size_t n = 0;
		for (auto& _move : _node)
		{
			auto _newBoard = _board;
			_newBoard.move(_move.move_);

			auto _score = alpha_beta_max(_newBoard, _move, _profile,
				_searchData.with_next_depth(), _alphaBeta);

			if (_score <= _alphaBeta.alpha)
			{
				return _alphaBeta.alpha; // fail hard alpha-cutoff
			};
			if (_score < _alphaBeta.beta)
			{
				_alphaBeta.beta = _score; // beta acts like min in MiniMax
			};

			++n;
		};

		return _alphaBeta.beta;
	};



	Rating alpha_beta(const Board& _board,
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
		_node.evaluate_next_with_board(_board, _profile, _searchData);
		
		if (_isMaximizingPlayer)
		{
			auto _value = 
				-Rating(std::numeric_limits<Rating>::infinity());

			size_t n = 0;
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

				++n;
			};

			return _value;
		}
		else
		{
			auto _value =
				Rating(std::numeric_limits<Rating>::infinity());

			size_t n = 0;
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
				++n;
			};

			return _value;
		};

		::abort();
	};


	void evaluate_next_full_move(const Board& _board, MoveTreeNode& _node,
		const MoveTreeProfile& _profile, MoveTreeSearchData _searchData)
	{
		// Node should be empty.
		SCREEPFISH_ASSERT(_node.empty());

		// Evaluate 2 half-moves deeper.
		_node.evaluate_next(_board, _profile, _searchData);
		_node.evaluate_next(_board, _profile, _searchData);


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

	void MoveTree::evalulate_next(MoveTreeSearchData _searchData, const MoveTreeProfile& _profile)
	{
		return this->evaluate_next(_searchData, _profile);
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
			//for (auto& _move : _root)
			//{
			//	if (_move.empty())
			//	{
			//		return _move.move;
			//	};
			//};

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

		{
			auto _alphaBeta = MoveTreeAlphaBeta();
			_alphaBeta.alpha = -std::numeric_limits<Rating>::infinity();
			_alphaBeta.beta = std::numeric_limits<Rating>::infinity();

			const auto _alphaBetaRating = alpha_beta(
				this->initial_board(), _root,
				_profile, _searchData, _alphaBeta, true);
			
			deep_eval(_root);
			this->resort_children(nullptr);
		};
		
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
