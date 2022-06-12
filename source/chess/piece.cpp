#include "piece.hpp"

#include "fen.hpp"

#include <ostream>

namespace chess
{
	std::ostream& operator<<(std::ostream& _ostr, const Piece& p)
	{
		const auto c = piece_to_char(p);
		_ostr.put(c);
		return _ostr;
	};

	std::ostream& operator<<(std::ostream& _ostr, const PieceMove& _value)
	{
		return _ostr << _value.from() << _value.to();
	};

	std::ostream& operator<<(std::ostream& _ostr, const Move& _value)
	{
		_ostr << _value.from() << _value.to();
		if (_value.promotion() != PieceType{})
		{
			char c = ' ';
			switch (_value.promotion())
			{
			case PieceType::queen:
				c = 'q';
				break;
			case PieceType::knight:
				c = 'n';
				break;
			case PieceType::rook:
				c = 'r';
				break;
			case PieceType::bishop:
				c = 'b';
				break;
			default:
				abort();
			};
			_ostr << c;
		};
		return _ostr;
	};

};