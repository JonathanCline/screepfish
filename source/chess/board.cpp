#include "board.hpp"

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

		// Loop over pieces and set attacking positions
		const auto pend = _board.pend();
		for (auto it = _board.pbegin(); it != pend; ++it)
		{
			auto& _piece = *it;
			if (_piece.color() == Color::white)
			{

			}
			else
			{

			};
		};
	};

	void BoardPieceAttackData::move(const BoardBase& _previousBoard, Move _move)
	{
		// TODO : Prevent full resync.
		auto nb = _previousBoard;
		nb.move(_move);
		this->sync(nb);
	};

}