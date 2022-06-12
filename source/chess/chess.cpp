#include "chess.hpp"

#include "fen.hpp"

#include <istream>
#include <ostream>
#include <string>
#include <array>


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

	std::ostream& operator<<(std::ostream& _ostr, const Board& _value)
	{
		std::array<std::array<char, 8>, 8> _grid{};
		for (auto& vx : _grid) { std::ranges::fill(vx, ' '); };

		for (auto& _piece : _value.pieces())
		{
			const auto f = _piece.file();
			const auto r = _piece.rank();
			
			auto x = jc::to_underlying(f);
			auto y = jc::to_underlying(r);
			auto c = ' ';

			switch (_piece.type())
			{
			case PieceType::pawn: c = 'P'; break;
			case PieceType::knight: c = 'N'; break;
			case PieceType::bishop: c = 'B'; break;
			case PieceType::rook: c = 'R'; break;
			case PieceType::queen: c = 'Q'; break;
			case PieceType::king: c = 'K'; break;
			default:
				continue;
				break;
			};

			if (_piece.color() == Color::black)
			{
				c = static_cast<char>(std::tolower(c));
			};

			_grid[static_cast<size_t>(7 - y)][x] = c;
		};

		_ostr << "+-+-+-+-+-+-+-+-+\n";
		for (auto& vx : _grid)
		{
			for (auto& c : vx)
			{
				_ostr.put('|');
				_ostr.put(c);
			};
			_ostr.put('|');
			_ostr.put('\n');
			_ostr << "+-+-+-+-+-+-+-+-+\n";
		};

		return _ostr;
	};

}