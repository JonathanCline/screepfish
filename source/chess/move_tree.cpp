#include "move_tree.hpp"


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
				const auto _rating = rate_move(_newBoard, *p, !this->move_played_by);

				// Assign values
				it->move_played_by = !this->move_played_by;
				it->move = RatedMove{ *p, _rating };

				// Next
				++it;
			};

			// Sort children by rating
			std::ranges::sort(this->responses, [](const MoveTreeNode& lhs, const MoveTreeNode& rhs) -> bool
				{
					return lhs.move.rating > rhs.move.rating;
				});
		}
		else
		{
			// Propogate to children
			for (auto& _child : this->responses)
			{
				_child.evaluate_next(_board);
			};
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
				const auto _rating = rate_move(_newBoard, *p, this->to_play);

				// Assign values
				it->move_played_by = this->to_play;
				it->move = RatedMove{ *p, _rating };

				// Next
				++it;
			};

			// Sort children by rating
			std::ranges::sort(this->moves, [](const MoveTreeNode& lhs, const MoveTreeNode& rhs) -> bool
				{
					return lhs.move.rating > rhs.move.rating;
				});
		}
		else
		{
			// Propogate to children
			for (auto& _child : this->moves)
			{
				_child.evaluate_next(_board);
			};
		};
	};

	Move MoveTree::best_move()
	{
		for (auto& c : this->moves)
		{
			auto& cr = c.responses.front().move.rating;



		};


		return this->moves.front().move.move;
	};

};
