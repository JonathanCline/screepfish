#include "board.hpp"

#include "precompute.hpp"

#include <ostream>

namespace chess
{
	std::ostream& operator<<(std::ostream& _ostr, const Board& _value)
	{
		return _ostr << _value.board();
	};
};

namespace chess
{
	void BoardPieceAttackData::clear()
	{
		this->wattack_.reset();
		this->battack_.reset();
	};

	void BoardPieceAttackData::sync(const BoardBase& _board)
	{
		// Clear old state
		this->clear();

		// Loop over pieces and set attacking positions.
		const auto pend = _board.pend();
		for (auto it = _board.pbegin(); it != pend; ++it)
		{
			const auto& _piece = *it;
			
			// Grab the attacking bitboard for the given player.
			auto& _attackBB =
				(_piece.color() == Color::white) ? this->wattack_ : this->battack_;

			// Determine attacking squares and add to bitboard.
			if (_piece == Piece::pawn)
			{
				const auto pa = get_pawn_attacking_squares(_piece.position(), _piece.color());
				_attackBB |= pa;
			};
		};
	};

	void BoardPieceAttackData::move(const BoardBase& _previousBoard, Move _move)
	{
		// TODO : Prevent full resync.

		if (_previousBoard.get_toplay() == Color::white)
		{

		}
		else
		{

		};

		auto nb = _previousBoard;
		nb.move(_move);
		this->sync(nb);
	};

}