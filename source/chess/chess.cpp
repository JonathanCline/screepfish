#include "chess.hpp"

#include <istream>
#include <ostream>
#include <string>
#include <array>

namespace chess
{
	std::ostream& operator<<(std::ostream& _ostr, const File& _value)
	{
		return _ostr << tochar(_value);
	};
	std::istream& operator>>(std::istream& _istr, File& _value)
	{
		const auto c = _istr.peek();
		if (c >= static_cast<int>('a') && c <= static_cast<int>('h'))
		{
			fromchar(static_cast<char>(c), _value);
			_istr.ignore();
		}
		else
		{
			_istr.setstate(std::ios_base::failbit);
		};
		return _istr;
	};

	std::ostream& operator<<(std::ostream& _ostr, const Rank& _value)
	{
		return _ostr << tochar(_value);
	};
	std::istream& operator>>(std::istream& _istr, Rank& _value)
	{
		const auto c = _istr.peek();
		if (c >= static_cast<int>('1') && c <= static_cast<int>('8'))
		{
			fromchar(static_cast<char>(c), _value);
			_istr.ignore();
		}
		else
		{
			_istr.setstate(std::ios_base::failbit);
		};
		return _istr;
	};

	std::ostream& operator<<(std::ostream& _ostr, const Position& _value)
	{
		return _ostr << _value.file() << _value.rank();
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

		for (auto& _piece : _value.pieces_)
		{
			auto x = jc::to_underlying(_piece.file());
			auto y = jc::to_underlying(_piece.rank());
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
				SCREEPFISH_UNREACHABLE; break;
			};

			if (_piece.color() == Color::black)
			{
				c = static_cast<char>(std::tolower(c));
			};

			_grid[static_cast<size_t>(7 - y)][x] = c;
		};

		for (auto& vx : _grid)
		{
			_ostr.write(vx.data(), vx.size());
			_ostr.put('\n');
		};

		return _ostr;
	};

};
