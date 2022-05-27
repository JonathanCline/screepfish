#include "fen.hpp"

#include "utility/string.hpp"
#include <charconv>

namespace chess
{
	inline char digit_to_char(int n)
	{
		switch (n)
		{
		case 0:
			return '0';
		case 1:
			return '1';
		case 2:
			return '2';
		case 3:
			return '3';
		case 4:
			return '4';
		case 5:
			return '5';
		case 6:
			return '6';
		case 7:
			return '7';
		case 8:
			return '8';
		case 9:
			return '9';
		default:
			abort();
			SCREEPFISH_UNREACHABLE;
		};
	};
	inline int char_to_digit(char c)
	{
		if (c < '0' || c > '9')
		{
			abort();
		};
		return c - '0';
	};

	inline char piece_to_char(PieceType p)
	{
		switch (p)
		{
		case PieceType::none:
			return ' ';
		case PieceType::pawn:
			return 'p';
		case PieceType::knight:
			return 'n';
		case PieceType::bishop:
			return 'b';
		case PieceType::rook:
			return 'r';
		case PieceType::queen:
			return 'q';
		case PieceType::king:
			return 'k';
		default:
			abort();
		};
	};
	inline char piece_to_char(Piece p)
	{
		auto c = piece_to_char(p.type());
		if (p.color() == Color::white)
		{
			c = (char)toupper(c);
		};
		return c;
	};





	inline std::optional<std::array<Piece, 8>> parse_rank(std::string_view _fen)
	{
		auto o = std::array<Piece, 8>{};
		auto it = o.begin();

		for (auto& c : _fen)
		{
			switch (c)
			{
			case 'p':
				*it = Piece(Piece::pawn, Color::black);
				++it;
				break;
			case 'P':
				*it = Piece(Piece::pawn, Color::white);
				++it;
				break;
			
			case 'b':
				*it = Piece(Piece::bishop, Color::black);
				++it;
				break;
			case 'B':
				*it = Piece(Piece::bishop, Color::white);
				++it;
				break;

			case 'n':
				*it = Piece(Piece::knight, Color::black);
				++it;
				break;
			case 'N':
				*it = Piece(Piece::knight, Color::white);
				++it;
				break;

			case 'r':
				*it = Piece(Piece::rook, Color::black);
				++it;
				break;
			case 'R':
				*it = Piece(Piece::rook, Color::white);
				++it;
				break;

			case 'q':
				*it = Piece(Piece::queen, Color::black);
				++it;
				break;
			case 'Q':
				*it = Piece(Piece::queen, Color::white);
				++it;
				break;

			case 'k':
				*it = Piece(Piece::king, Color::black);
				++it;
				break;
			case 'K':
				*it = Piece(Piece::king, Color::white);
				++it;
				break;


			case '1':
				it = it + 1;
				break;
			case '2':
				it = it + 2;
				break;
			case '3':
				it = it + 3;
				break;
			case '4':
				it = it + 4;
				break;
			case '5':
				it = it + 5;
				break;
			case '6':
				it = it + 6;
				break;
			case '7':
				it = it + 7;
				break;
			case '8':
				it = it + 8;
				break;
			default:
				return std::nullopt;
			};
		};

		return o;
	};


	inline bool parse_next_fen_rank(std::string_view _fen, Board& _board, size_t& _offset, Rank& _rank)
	{
		const auto p = _fen.find_first_of("/ ", _offset);
		if (p == _fen.npos) { return false; };
		const auto s = _fen.substr(_offset, p - _offset);
		auto r = parse_rank(s);
		if (!r) { return false; };
		auto f = File::a;
		for (auto& v : *r)
		{
			if (v)
			{
				_board.new_piece(v, Position(f, _rank));
			};
			f = f + 1;
		};
		_rank = _rank - 1;
		_offset = p + 1;
		return true;
	};



	std::optional<Board> parse_fen(const std::string_view _inFen)
	{
		// Copy fen for parsing.
		auto _fen = _inFen;

		auto _board = Board();
		_board.clear();

		// Parse ranks
		{
			size_t _offset = 0;
			auto _rank = Rank::r8;
			for (int n = 0; n != 8; ++n)
			{
				if (!parse_next_fen_rank(_fen, _board, _offset, _rank)) { return std::nullopt; };
			};
			_fen.remove_prefix(_offset);
			_fen = str::lstrip(_fen);
		};

		// Parse toplay char
		if (_fen.starts_with('w'))
		{
			_board.set_toplay(Color::white);
		}
		else if (_fen.starts_with('b'))
		{
			_board.set_toplay(Color::black);
		}
		else
		{
			return std::nullopt;
		};
		_fen.remove_prefix(1); _fen = str::lstrip(_fen);

		// Parse castling
		if (_fen.starts_with('-'))
		{
			_fen.remove_prefix(1);
			_fen = str::lstrip(_fen);
		}
		else
		{
			int n = 0;
			for (auto& c : _fen)
			{
				switch (c)
				{
				case 'k':
					_board.set_castle_kingside_flag(Color::black, true);
					++n;
					break;
				case 'K':
					_board.set_castle_kingside_flag(Color::white, true);
					++n;
					break;
				case 'q':
					_board.set_castle_queenside_flag(Color::black, true);
					++n;
					break;
				case 'Q':
					_board.set_castle_queenside_flag(Color::white, true);
					++n;
					break;
				case ' ':
					break;
				default:
					return std::nullopt;
				};

				if (c == ' ')
				{
					break;
				};
			};

			_fen.remove_prefix(n);
			_fen = str::lstrip(_fen);
		};

		// Parse pissant
		if (_fen.starts_with('-'))
		{
			_fen.remove_prefix(1);
			_fen = str::lstrip(_fen);
		}
		else
		{
			File f;
			Rank r;
			fromchar(_fen.front(), f);
			_fen.remove_prefix(1);

			fromchar(_fen.front(), r);
			_fen.remove_prefix(1);

			_board.set_enpassant_target((f, r));
			_fen = str::lstrip(_fen);
		};

		// Parse half move
		{
			auto p =  _fen.find(' ');
			if (p == _fen.npos) { return std::nullopt; };
			uint16_t v;
			const auto r = std::from_chars(_fen.data(), _fen.data() + p, v);
			if (r.ec != std::errc{}) { return std::nullopt; };
			_board.set_half_move_count(v);
			_fen.remove_prefix(p);
			_fen = str::lstrip(_fen);
		};

		// Parse full move
		{
			auto p = _fen.find(' ');
			if (p == _fen.npos) { p = _fen.size(); };
			uint16_t v;
			const auto r = std::from_chars(_fen.data(), _fen.data() + p, v);
			if (r.ec != std::errc{}) { return std::nullopt; };
			_board.set_full_move_count(v);
			_fen = str::lstrip(_fen);
		};

		return _board;
	};

	std::string get_fen(const Board& _board)
	{
		std::string s{};
	
		// Add ranks
		for (auto& r : rev_ranks_v)
		{
			if (!s.empty())
			{
				s += '/';
			};

			int ec = 0;
			for(auto& f : files_v)
			{
				const auto p = _board.get(f, r);
				if (!p)
				{
					++ec;
				}
				else
				{
					if (ec != 0)
					{
						s += digit_to_char(ec);
					};
					s += piece_to_char(p);
					ec = 0;
				};
			};

			if (ec != 0)
			{
				s += digit_to_char(ec);
			};
		};
		s += ' ';

		// Set to move
		if (_board.get_toplay() == Color::white)
		{
			s += 'w';
		}
		else
		{
			s += 'b';
		};
		s += ' ';

		if (_board.get_castle_kingside_flag(Color::white))
		{
			s += 'K';
		};
		if (_board.get_castle_queenside_flag(Color::white))
		{
			s += 'Q';
		};
		if (_board.get_castle_kingside_flag(Color::black))
		{
			s += 'k';
		};
		if (_board.get_castle_queenside_flag(Color::black))
		{
			s += 'q';
		};
		if (s.back() == ' ')
		{
			s += '-';
		};

		s += ' ';
		
		if (_board.has_enpassant_target())
		{
			const auto p = _board.enpassant_target();
			s += tochar(p.file());
			s += tochar(p.rank());
		}
		else
		{
			s += '-';
		};
		s += ' ';

		// half move
		s.append(std::to_string(_board.get_half_move_count()));
		s += ' ';

		// full move
		s.append(std::to_string(_board.get_full_move_count()));

		return s;
	};
};