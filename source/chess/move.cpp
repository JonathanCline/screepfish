#include "move.hpp"

#include <iostream>

namespace chess
{
	namespace
	{
		constexpr auto CHECKMATE_RATING = 100000.0f;
		constexpr auto BLOCKED_QUEEN_RATING = 0.001f;
		constexpr auto BLOCKED_ROOK_RATING = 0.001f;
		constexpr auto BLOCKED_BISHOP_RATING = 0.001f;
		constexpr auto PAWN_PUSH_RATING = 0.001f;
		constexpr auto CASTLE_ABILITY_RATING = 0.001f;
		constexpr auto DEVELOPMENT_RATING = 0.005f;
	};


	inline chess::Position find_next_piece_in_direction(const chess::Board& _board, const chess::Position& _startPos,
		const int df, const int dr)
	{
		using namespace chess;

		auto f = _startPos.file() + df;
		auto r = _startPos.rank() + dr;

		while (f <= File::h && r <= Rank::r8)
		{
			const auto p = (f, r);
			if (!_board.is_empty(p))
			{
				return p;
			};

			f += df;
			r += dr;
		};

		return _startPos;
	};

	inline void find_legal_positions_in_direction(const chess::Board& _board, const chess::Position& _startPos,
		chess::Color _myColor, const int df, const int dr, MoveBuffer& _buffer)
	{
		using namespace chess;

		auto f = _startPos.file() + df;
		auto r = _startPos.rank() + dr;

		while (f <= File::h && r <= Rank::r8)
		{
			const auto pos = (f, r);
			if (const auto p = _board.get(pos); p)
			{
				if (p.color() == _myColor)
				{
					break;
				}
				else
				{
					_buffer.write(_startPos, pos);
					break;
				};
			}
			else
			{
				_buffer.write(_startPos, pos);
			};

			f += df;
			r += dr;
		};
	};


	
	consteval BitBoardCX compute_pawn_attack_squares(Position _pos, Color _color)
	{
		auto bb = BitBoardCX();
		if (_pos.file() != File::a)
		{
			if (_color == Color::white)
			{
				bb.set(next(_pos, -1, 1));
			}
			else
			{
				bb.set(next(_pos, -1, -1));
			};
		};
		if (_pos.file() != File::h)
		{
			if (_color == Color::white)
			{
				bb.set(next(_pos, 1, 1));
			}
			else
			{
				bb.set(next(_pos, 1, -1));
			};
		};
		return bb;
	};
	consteval auto compute_pawn_attack_squares(Color _color)
	{
		std::array<BitBoardCX, 64> bbs{};
		auto it = bbs.begin();
		for (auto& v : positions_v)
		{
			if (v.rank() == Rank::r1 || v.rank() == Rank::r8)
			{
				++it;
				continue;
			};
			*it = compute_pawn_attack_squares(v, _color);
			++it;
		};
		return bbs;
	};

	// Precompute attack squares
	constexpr inline auto white_pawn_attack_squares_v = compute_pawn_attack_squares(Color::white);
	constexpr inline auto black_pawn_attack_squares_v = compute_pawn_attack_squares(Color::black);

	constexpr inline auto get_pawn_attacking_squares(Position _pos, Color _color)
	{
		if (_color == Color::white)
		{
			return white_pawn_attack_squares_v[static_cast<size_t>(_pos)];
		}
		else
		{
			return black_pawn_attack_squares_v[static_cast<size_t>(_pos)];
		};
	};
	
	
	consteval BitBoardCX compute_pawn_move_squares(Position _pos, Color _color)
	{
		auto bb = BitBoardCX();
		if (_color == Color::white)
		{
			if (_pos.rank() == Rank::r2)
			{
				bb.set(_pos.file(), Rank::r4);
			};
			bb.set(next((_pos.file(), _pos.rank()), 0, 1));
		}
		else
		{
			if (_pos.rank() == Rank::r7)
			{
				bb.set(_pos.file(), Rank::r5);
			};
			bb.set(next((_pos.file(), _pos.rank()), 0, -1));
		};

		return bb;
	};
	consteval auto compute_pawn_move_squares(Color _color)
	{
		std::array<BitBoardCX, 64> bbs{};
		auto it = bbs.begin();
		for (auto& v : positions_v)
		{
			if (v.rank() == Rank::r1 || v.rank() == Rank::r8)
			{
				++it;
				continue;
			};
			*it = compute_pawn_move_squares(v, _color);
			++it;
		};
		return bbs;
	};

	// Precompute move squares
	constexpr inline auto white_pawn_move_squares_v = compute_pawn_move_squares(Color::white);
	constexpr inline auto black_pawn_move_squares_v = compute_pawn_move_squares(Color::black);

	constexpr inline auto get_pawn_movement_squares(Position _pos, Color _color)
	{
		if (_color == Color::white)
		{
			return white_pawn_move_squares_v[static_cast<size_t>(_pos)];
		}
		else
		{
			return black_pawn_move_squares_v[static_cast<size_t>(_pos)];
		};
	};



	consteval BitBoardCX compute_knight_attack_squares(Position _pos)
	{
		auto bb = BitBoardCX();
		constexpr auto _deltaPairs = std::array
		{
			std::pair{ 1, 2 },
			std::pair{ 1, -2 },

			std::pair{ 2, 1 },
			std::pair{ 2, -1 },

			std::pair{ -1, 2 },
			std::pair{ -1, -2 },

			std::pair{ -2, -1 },
			std::pair{ -2, 1 },
		};

		bool _possible = false;
		Position _nextPos{};
		for (const auto& [df, dr] : _deltaPairs)
		{
			_nextPos = trynext(_pos, df, dr, _possible);
			if (_possible)
			{
				bb.set(_nextPos);
			};
		};

		return bb;
	};
	consteval auto compute_knight_attack_squares()
	{
		std::array<BitBoardCX, 64> bbs{};
		auto it = bbs.begin();
		for (auto& v : positions_v)
		{
			*it = compute_knight_attack_squares(v);
			++it;
		};
		return bbs;
	};

	// Precompute attack squares
	constexpr inline auto knight_attack_squares_v = compute_knight_attack_squares();
	constexpr inline auto get_knight_attack_squares(Position _pos)
	{
		return knight_attack_squares_v[static_cast<size_t>(_pos)];
	};





	bool is_piece_attacked_by_pawn(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
	{
		using namespace chess;

		// Bitboard with squares pawn is attacking
		const auto _attackingBB = get_pawn_attacking_squares(_byPiece.position(), _byPiece.color());
		
		// If bit is set, piece is being attacked
		return _attackingBB.test(_piece.position());
	};
	bool is_piece_attacked_by_knight(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
	{
		using namespace chess;
		const auto _attackBB = get_knight_attack_squares(_byPiece.position());
		return _attackBB.test(_piece.position());
	};
	bool is_piece_attacked_by_bishop(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
	{
		using namespace chess;

		// Calculate the offset from the piece to the bishop.
		const auto _position = _byPiece.position();
		const auto _offset = _piece.position() - _position;

		// If both are the same magnitude then the bishop is on the same diagonal.
		if (abs(_offset.delta_file()) == abs(_offset.delta_rank()))
		{
			const auto _dir = Offset(Direction(_offset));

			auto f = _byPiece.file();
			auto r = _byPiece.rank();

			while (true)
			{
				f += _dir.delta_file();
				r += _dir.delta_rank();
				if (f > File::h || r > Rank::r8)
				{
					break;
				};

				const auto _nextPosition = Position(f, r);
				if (_nextPosition == _piece.position())
				{
					return true;
				}
				else if (!_board.is_empty(_nextPosition))
				{
					break;
				};
			};
		};

		return false;
	};
	bool is_piece_attacked_by_rook(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
	{
		using namespace chess;

		const auto _position = _byPiece.position();
		const auto _offset = _piece.position() - _position;

		if (!((_offset.delta_rank() == 0) ^ (_offset.delta_file() == 0)))
		{
			// Not on same file / rank
			return false;
		};

		constexpr auto _directionPairs = std::array
		{
			std::pair{  0,  1 },
			std::pair{  0, -1 },
			std::pair{  1,  0 },
			std::pair{ -1,  0 },
		};

		for (const auto& _direction : _directionPairs)
		{
			auto [df, dr] = _direction;
			auto f = _byPiece.file();
			auto r = _byPiece.rank();

			while (true)
			{
				f += df;
				r += dr;
				if (f > File::h || r > Rank::r8)
				{
					break;
				};

				const auto _nextPosition = Position(f, r);
				if (_nextPosition == _piece.position())
				{
					return true;
				}
				else if (!_board.is_empty(_nextPosition))
				{
					break;
				};
			};
		};
		return false;
	};
	bool is_piece_attacked_by_queen(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
	{
		using namespace chess;

		const auto _position = _byPiece.position();
		auto _nextPosition = Position();
		bool _possible = true;

		constexpr auto _directionPairs = std::array
		{
			std::pair{  1,  0 },
			std::pair{ -1,  0 },
			std::pair{  0,  1 },
			std::pair{  0, -1 },

			std::pair{  1,  1 },
			std::pair{ -1,  1 },
			std::pair{  1, -1 },
			std::pair{ -1, -1 },
		};

		for (const auto& _direction : _directionPairs)
		{
			int n = 1;
			while (true)
			{
				auto [df, dr] = _direction;
				df *= n;
				dr *= n;

				_nextPosition = trynext(_position, df, dr, _possible);
				if (_possible)
				{
					if (_nextPosition == _piece.position())
					{
						return true;
					};

					if (!_board.is_empty(_nextPosition))
					{
						break;
					};
				}
				else
				{
					break;
				};

				++n;
			};
		};
		return false;
	};
	bool is_piece_attacked_by_king(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
	{
		auto fd = std::abs((int)_piece.file() - (int)_byPiece.file());
		auto rd = std::abs((int)_piece.rank() - (int)_byPiece.rank());
		return (fd <= 1 && rd <= 1);
	};

	bool is_piece_attacked(const chess::Board& _board, const chess::BoardPiece& _piece, bool _inCheck)
	{
		std::array<Move, 64> _data{};
		auto _buffer = MoveBuffer(_data);
		const auto bp = _buffer.head();

		get_piece_attacked_from_moves(_board, _piece, _buffer, _inCheck);
		if (_buffer.head() != bp)
		{
			return true;
		}
		else
		{
			return false;
		};
	};



	void get_piece_attacks_with_pawn(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer)
	{
		if (is_piece_attacked_by_pawn(_board, _piece, _byPiece))
		{
			_buffer.write(_byPiece.position(), _piece.position());
		};
	};
	void get_piece_attacks_with_knight(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer)
	{
		if (is_piece_attacked_by_knight(_board, _piece, _byPiece))
		{
			_buffer.write(_byPiece.position(), _piece.position());
		};
	};
	void get_piece_attacks_with_bishop(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer)
	{
		if (is_piece_attacked_by_bishop(_board, _piece, _byPiece))
		{
			_buffer.write(_byPiece.position(), _piece.position());
		};
	};
	void get_piece_attacks_with_rook(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer)
	{
		if (is_piece_attacked_by_rook(_board, _piece, _byPiece))
		{
			_buffer.write(_byPiece.position(), _piece.position());
		};
	};
	void get_piece_attacks_with_queen(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer)
	{
		if (is_piece_attacked_by_queen(_board, _piece, _byPiece))
		{
			_buffer.write(_byPiece.position(), _piece.position());
		};
	};
	void get_piece_attacks_with_king(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer)
	{
		if (is_piece_attacked_by_king(_board, _piece, _byPiece))
		{
			_buffer.write(_byPiece.position(), _piece.position());
		};
	};

	void get_piece_attacked_from_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _outBuffer, bool _inCheck)
	{
		using namespace chess;

		std::array<Move, 32> _bufferData{};
		MoveBuffer _buffer{ _bufferData.data(), _bufferData.data() + _bufferData.size() };
		const auto bb = _buffer.head();

		// Quicker pawn check
		if (_piece.color() == Color::white)
		{
			if (_piece.rank() < Rank::r7)
			{
				if (_piece.file() != File::a)
				{
					const auto np = next(_piece.position(), -1, 1);
					if (_board.get(np) == Piece(Piece::pawn, Color::black))
					{
						_buffer.write(np, _piece.position());
					};
				};
				if (_piece.file() != File::h)
				{
					const auto np = next(_piece.position(), 1, 1);
					if (_board.get(np) == Piece(Piece::pawn, Color::black))
					{
						_buffer.write(np, _piece.position());
					};
				};
			};
		}
		else
		{
			if (_piece.rank() > Rank::r2)
			{
				if (_piece.file() != File::a)
				{
					const auto np = next(_piece.position(), -1, -1);
					if (_board.get(np) == Piece(Piece::pawn, Color::white))
					{
						_buffer.write(np, _piece.position());
					};
				};
				if (_piece.file() != File::h)
				{
					const auto np = next(_piece.position(), 1, -1);
					if (_board.get(np) == Piece(Piece::pawn, Color::white))
					{
						_buffer.write(np, _piece.position());
					};
				};
			};
		};

		const auto _end = _board.pend();
		for (auto it = _board.pbegin(); it != _end; ++it)
		{
			auto& p = *it;
			if (p.color() != _piece.color())
			{
				switch (p.type())
				{
				case PieceType::knight:
					get_piece_attacks_with_knight(_board, _piece, p, _buffer);
					break;
				case PieceType::bishop:
					get_piece_attacks_with_bishop(_board, _piece, p, _buffer);
					break;
				case PieceType::rook:
					get_piece_attacks_with_rook(_board, _piece, p, _buffer);
					break;
				case PieceType::queen:
					get_piece_attacks_with_queen(_board, _piece, p, _buffer);
					break;
				case PieceType::king:
					get_piece_attacks_with_king(_board, _piece, p, _buffer);
					break;
				case PieceType::pawn:
					get_piece_attacks_with_pawn(_board, _piece, p, _buffer);
					break;
				default:
					break;
				};
			};
		};

		const auto be = _buffer.head();
		for (auto bp = bb; bp != be; ++bp)
		{
			auto b = _board;
			b.move(*bp);

			if (_piece.type() == Piece::king || !is_check(b, !_piece.color()))
			{
				_outBuffer.write(*bp);
			};
		};
	};




	void get_pawn_moves(const chess::Board& _board, const chess::BoardPiece& _piece,
		MoveBuffer& _buffer, const bool _isCheck)
	{
		using namespace chess;

		const auto _position = _piece.position();
		const auto _rank = _position.rank();
		const auto _file = _position.file();

		// Exit early if at end of board
		if (_rank == Rank::r1 || _rank == Rank::r8)
		{
			return;
		};

		// Change in rank, depends on piece color
		int _deltaRank = 1;

		// 2X delta rank for initial pawn move
		bool _doubleDeltaRank = false;

		if (_piece.color() == Color::white)
		{
			if (_rank == Rank::r2)
			{
				_doubleDeltaRank = true;
			};
			_deltaRank = 1;
		}
		else
		{
			if (_rank == Rank::r7)
			{
				_doubleDeltaRank = true;
			};
			_deltaRank = -1;
		};

		{
			bool _possible = false;
			const auto _newPosOne = next((_file, _rank), 0, _deltaRank);

			if (_board.is_empty(_newPosOne))
			{
				if (!_doubleDeltaRank && (_newPosOne.rank() == Rank::r8 || _newPosOne.rank() == Rank::r1))
				{
					_buffer.write(Move(_position, _newPosOne, PieceType::bishop));
					_buffer.write(Move(_position, _newPosOne, PieceType::rook));
					_buffer.write(Move(_position, _newPosOne, PieceType::knight));
					_buffer.write(Move(_position, _newPosOne, PieceType::queen));
				}
				else
				{
					_buffer.write(Move(_position, _newPosOne));
				};

				// Can move x2 if not blocked
				if (_doubleDeltaRank)
				{
					const auto _newPosTwo = next((_file, _rank), 0, _deltaRank * 2);
					if (_board.is_empty(_newPosTwo))
					{
						_buffer.write(Move(_position, _newPosTwo));
					};
				};
			};
		};

		// Can capture diagonally
		if (_file != File::h)
		{
			// Check for piece to capture
			const auto _newPos = next(_position, 1, _deltaRank);
			if ((_board.has_enpassant_target() && _board.enpassant_target() == _newPos)
				|| _board.has_enemy_piece(_newPos, _piece.color()))
			{
				if (_newPos.rank() == Rank::r8 || _newPos.rank() == Rank::r1)
				{
					_buffer.write(Move(_position, _newPos, PieceType::bishop));
					_buffer.write(Move(_position, _newPos, PieceType::rook));
					_buffer.write(Move(_position, _newPos, PieceType::knight));
					_buffer.write(Move(_position, _newPos, PieceType::queen));
				}
				else
				{
					_buffer.write(Move(_position, _newPos));
				};
			};
		};
		if (_file != File::a)
		{
			// Check for piece to capture
			const auto _newPos = next(_position, -1, _deltaRank);
			if ((_board.has_enpassant_target() && _board.enpassant_target() == _newPos)
				|| _board.has_enemy_piece(_newPos, _piece.color()))
			{
				if (_newPos.rank() == Rank::r8 || _newPos.rank() == Rank::r1)
				{
					_buffer.write(Move(_position, _newPos, PieceType::bishop));
					_buffer.write(Move(_position, _newPos, PieceType::rook));
					_buffer.write(Move(_position, _newPos, PieceType::knight));
					_buffer.write(Move(_position, _newPos, PieceType::queen));
				}
				else
				{
					_buffer.write(Move(_position, _newPos));
				};
			};
		};
	};
	void get_rook_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck)
	{
		using namespace chess;

		const auto _position = _piece.position();
		const auto _rank = _position.rank();
		const auto _file = _position.file();

		const auto _directions = std::array
		{
			std::pair{ 0, 1 },
			std::pair{ 0, -1 },
			std::pair{ 1, 0 },
			std::pair{ -1, 0 }
		};
		
		for (auto& _direction : _directions)
		{
			find_legal_positions_in_direction(_board, _piece.position(), _piece.color(), _direction.first, _direction.second, _buffer);
		};
	};
	void get_knight_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck)
	{
		using namespace chess;

		const auto _position = _piece.position();
		auto _nextPosition = Position();
		bool _possible = true;

		constexpr auto _deltaPairs = std::array
		{
			std::pair{ 1, 2 },
			std::pair{ 1, -2 },

			std::pair{ 2, 1 },
			std::pair{ 2, -1 },

			std::pair{ -1, 2 },
			std::pair{ -1, -2 },

			std::pair{ -2, -1 },
			std::pair{ -2, 1 },
		};

		for (const auto& [df, dr] : _deltaPairs)
		{
			_nextPosition = trynext(_position, df, dr, _possible);
			if (_possible && _board.has_enemy_piece_or_empty(_nextPosition, _piece.color()))
			{
				_buffer.write(_position, _nextPosition);
			};
		};
	};
	void get_king_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck)
	{
		using namespace chess;

		const auto _position = _piece.position();
		auto _nextPosition = Position();
		bool _possible = true;

		constexpr auto _deltaPairs = std::array
		{
			std::pair{ -1, -1 },
			std::pair{ -1,  0 },
			std::pair{ -1,  1 },
			std::pair{  0, -1 },

			std::pair{  0,  1 },
			std::pair{  1, -1 },
			std::pair{  1,  0 },
			std::pair{  1,  1 },
		};

		for (const auto& [df, dr] : _deltaPairs)
		{
			_nextPosition = trynext(_position, df, dr, _possible);
			if (_possible && _board.has_enemy_piece_or_empty(_nextPosition, _piece.color()))
			{
				_buffer.write(_position, _nextPosition);
			};
		};

		if (can_castle_kingside(_board, _piece.color()))
		{
			Position _dest{};
			if (_piece.color() == Color::white)
			{
				_dest = Position(File::g, Rank::r1);
			}
			else
			{
				_dest = Position(File::g, Rank::r8);
			};
			_buffer.write(_position, _dest);
		}
		if (can_castle_queenside(_board, _piece.color()))
		{
			Position _dest{};
			if (_piece.color() == Color::white)
			{
				_dest = Position(File::c, Rank::r1);
			}
			else
			{
				_dest = Position(File::c, Rank::r8);
			};
			_buffer.write(_position, _dest);
		};
	};
	void get_bishop_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck)
	{
		using namespace chess;

		const auto _position = _piece.position();
		auto _nextPosition = Position();
		bool _possible = true;

		constexpr auto _directionPairs = std::array
		{
			std::pair{  1,  1 },
			std::pair{ -1,  1 },
			std::pair{  1, -1 },
			std::pair{ -1, -1 },
		};

		for (const auto& _direction : _directionPairs)
		{
			find_legal_positions_in_direction(_board, _piece.position(), _piece.color(), _direction.first, _direction.second, _buffer);
		};
	};
	void get_queen_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck)
	{
		using namespace chess;

		const auto _position = _piece.position();
		auto _nextPosition = Position();
		bool _possible = true;

		constexpr auto _directionPairs = std::array
		{
			std::pair{  1,  0 },
			std::pair{ -1,  0 },
			std::pair{  0,  1 },
			std::pair{  0, -1 },
			std::pair{  1,  1 },
			std::pair{ -1,  1 },
			std::pair{  1, -1 },
			std::pair{ -1, -1 },
		};

		for (const auto& _direction : _directionPairs)
		{
			find_legal_positions_in_direction(_board, _piece.position(), _piece.color(), _direction.first, _direction.second, _buffer);
		};
	};

	void get_piece_moves(const chess::Board& _board, const chess::BoardPiece& _piece,
		MoveBuffer& _buffer, const bool _isCheck)
	{
		using namespace chess;

		const auto _piecePos = _piece.position();
		switch (_piece.type())
		{
		case PieceType::pawn:
		{
			get_pawn_moves(_board, _piece, _buffer, _isCheck);
		};
		break;
		case PieceType::rook:
		{
			get_rook_moves(_board, _piece, _buffer, _isCheck);
		};
		break;
		case PieceType::knight:
		{
			get_knight_moves(_board, _piece, _buffer, _isCheck);
		};
		break;
		case PieceType::king:
		{
			get_king_moves(_board, _piece, _buffer, _isCheck);
		};
		break;
		case PieceType::bishop:
		{
			get_bishop_moves(_board, _piece, _buffer, _isCheck);
		};
		break;
		case PieceType::queen:
		{
			get_queen_moves(_board, _piece, _buffer, _isCheck);
		};
		break;
		};
	};

	void get_moves(const chess::Board& _board, const chess::Color _forPlayer, MoveBuffer& _buffer, const bool _isCheck)
	{
		static thread_local auto _bufferData = std::array<chess::Move, 256>{};
		auto _movesBuffer = MoveBuffer(_bufferData.data(), _bufferData.data() + _bufferData.size());

		using namespace chess;

		if (_board.pfind(Piece::king, _forPlayer) == _board.pend())
		{
			return;
		};

		const auto _bufferStart = _movesBuffer.head();
		{
			const auto _end = _board.pend();
			for (auto it = _board.pbegin(); it != _end; ++it)
			{
				if (it->color() == _forPlayer)
				{
					get_piece_moves(_board, *it, _movesBuffer, _isCheck);
				};
			};
		};
		const auto _bufferEnd = _movesBuffer.head();

		for (auto p = _bufferStart; p != _bufferEnd; ++p)
		{
			auto nb = _board;
			nb.move(*p);
			if (!is_check(nb, _forPlayer))
			{
				_buffer.write(*p);
			};
		};
	};



	bool can_castle_kingside(const chess::Board& _board, chess::Color _player)
	{
		if (!_board.get_castle_kingside_flag(_player))
		{
			return false;
		};

		// Check the squares that castling would occur across.
		{
			auto _checkSquares = std::array<Position, 2>{};
			if (_player == Color::white)
			{
				_checkSquares[0] = (File::f, Rank::r1);
				_checkSquares[1] = (File::g, Rank::r1);
			}
			else
			{
				_checkSquares[0] = (File::f, Rank::r8);
				_checkSquares[1] = (File::g, Rank::r8);
			};

			// Cannot castle through other pieces.
			for (auto& v : _checkSquares)
			{
				if (!_board.is_empty(v))
				{
					return false;
				};
			};

			// Cannot castle out of check
			if (is_check(_board, _player))
			{
				return false;
			};

			// Cannot castle through / into check
			for (auto& v : _checkSquares)
			{
				auto b = _board;
				b.move(b.get_king(_player).position(), v);
				if (is_check(b, _player))
				{
					return false;
				};
			};
		};

		return true;
	};
	bool can_castle_queenside(const chess::Board& _board, chess::Color _player)
	{
		if (!_board.get_castle_queenside_flag(_player))
		{
			return false;
		};

		// Check the squares that castling would occur across.
		{
			auto _checkSquares = std::array<Position, 3>{};
			if (_player == Color::white)
			{
				_checkSquares[0] = (File::d, Rank::r1);
				_checkSquares[1] = (File::c, Rank::r1);
				_checkSquares[2] = (File::b, Rank::r1);
			}
			else
			{
				_checkSquares[0] = (File::d, Rank::r8);
				_checkSquares[1] = (File::c, Rank::r8);
				_checkSquares[2] = (File::b, Rank::r8);
			};

			// Cannot castle through other pieces.
			for (auto& v : _checkSquares)
			{
				if (!_board.is_empty(v))
				{
					return false;
				};
			};

			// Cannot castle out of check
			if (is_check(_board, _player))
			{
				return false;
			};

			// Cannot castle through / into check
			for (size_t n = 0; n != 2; ++n)
			{
				auto& v = _checkSquares[n];
				auto b = _board;
				b.move(b.get_king(_player).position(), v);
				if (is_check(b, _player))
				{
					return false;
				};
			};
		};

		return true;
	};




	struct Neighbors
	{
		std::array<Position, 8> neighbors{};
		size_t count = 0;

		std::array<Position, 4> rook_{};
		size_t rook_count_ = 0;

		std::array<Position, 4> bishop_{};
		size_t bishop_count_ = 0;

		constexpr std::span<const Position> rook() const
		{
			return std::span<const Position>(this->rook_.data(), this->rook_count_);
		};
		constexpr std::span<const Position> bishop() const
		{
			return std::span<const Position>(this->bishop_.data(), this->bishop_count_);
		};

		constexpr operator std::span<const Position>() const noexcept
		{
			return std::span<const Position>(this->neighbors.data(), this->count);
		};

		constexpr void append(Position p)
		{
			this->neighbors[this->count++] = p;
		};
		constexpr void rook_append(Position p)
		{
			this->rook_[this->rook_count_++] = p;
		};
		constexpr void bishop_append(Position p)
		{
			this->bishop_[this->bishop_count_++] = p;
		};

		constexpr Neighbors() = default;
	};

	consteval Neighbors find_neighbors(Position _pos)
	{
		auto _neighbors = Neighbors();

		constexpr auto _offsets = std::array
		{
			std::pair{ 0, 1 },
			std::pair{ 0, -1 },
			std::pair{ 1, 0 },
			std::pair{ -1, 0 },
			std::pair{ 1, 1 },
			std::pair{ -1, 1 },
			std::pair{ 1, -1 },
			std::pair{ -1, -1 },
		};

		for (const auto [df, dr] : _offsets)
		{
			bool _possible = false;
			const auto p = trynext(_pos, df, dr, _possible);
			if (_possible)
			{
				_neighbors.append(p);
				if (df == 0 || dr == 0)
				{
					_neighbors.rook_append(p);
				}
				else
				{
					_neighbors.bishop_append(p);
				};
			};
		};

		return _neighbors;
	};
	consteval auto precompute_neighbors()
	{
		std::array<Neighbors, 64> o{};
		auto it = o.begin();
		for (auto& v : positions_v)
		{
			*it = find_neighbors(v);
			++it;
		};
		return o;
	};

	constexpr inline auto neighbors_v = precompute_neighbors();



	std::span<const Position> get_surrounding_positions(Position _pos)
	{
		return neighbors_v[static_cast<size_t>(_pos)];
	};
	std::span<const Position> get_surrounding_positions_for_rook(Position _pos)
	{
		return neighbors_v[static_cast<size_t>(_pos)].rook();
	};
	std::span<const Position> get_surrounding_positions_for_bishop(Position _pos)
	{
		return neighbors_v[static_cast<size_t>(_pos)].bishop();
	};



	bool is_queen_blocked(const Board& _board, const Position _pos, Color _color)
	{
		const auto _spos = get_surrounding_positions(_pos);
		for (auto& o : _spos)
		{
			if (_board.has_enemy_piece_or_empty(o, _color))
			{
				return false;
			};
		};
		return true;
	};
	bool is_rook_blocked(const Board& _board, const Position _pos, Color _color)
	{
		const auto _spos = get_surrounding_positions_for_rook(_pos);
		for (auto& o : _spos)
		{
			if (_board.has_enemy_piece_or_empty(o, _color))
			{
				return false;
			};
		};
		return true;
	};
	bool is_bishop_blocked(const Board& _board, Position _pos, Color _color)
	{
		const auto _spos = get_surrounding_positions_for_bishop(_pos);
		for (auto& o : _spos)
		{
			if (_board.has_enemy_piece_or_empty(o, _color))
			{
				return false;
			};
		};
		return true;
	};



	constexpr BitBoardCX make_rank_bits(Rank _rank)
	{
		auto bb = BitBoardCX();
		for (auto& v : files_v)
		{
			bb.set(v, _rank);
		};
		return bb;
	};
	constexpr BitBoardCX make_file_bits(File _file)
	{
		auto bb = BitBoardCX();
		for (auto& v : ranks_v)
		{
			bb.set(_file, v);
		};
		return bb;
	};
	constexpr BitBoardCX make_file_bits(File _file, Rank _min, Rank _max)
	{
		auto bb = BitBoardCX();
		for (Rank r = _min; r <= _max; r += 1)
		{
			bb.set(_file, r);
		};
		return bb;
	};

	constexpr BitBoardCX make_bits_in_direction(Position _startPos, int df, int dr)
	{
		auto bb = BitBoardCX();

		bool _possible = false;
		auto _nextPos = trynext(_startPos, df, dr, _possible);
		while (_possible)
		{
			bb.set(_nextPos);
			_nextPos = trynext(_nextPos, df, dr, _possible);
		};
		return bb;
	};
	constexpr BitBoardCX make_diagonal_bits(Position _pos)
	{
		auto bb = BitBoardCX();
		const auto _directions = std::array
		{
			std::pair{ 1, 1 },
			std::pair{ 1, -1 },
			std::pair{ -1, 1 },
			std::pair{ -1, -1 }
		};
		for (auto& v : _directions)
		{
			bb |= make_bits_in_direction(_pos, v.first, v.second);
		};
		return bb;
	};



	consteval BitBoardCX calculate_threat_positions(Position _pos)
	{
		auto bb = BitBoardCX();

		// <--------->
		bb |= make_rank_bits(_pos.rank());
	
		// ^ 
		// |
		// |
		// v
		bb |= make_file_bits(_pos.file());

		// Diagonals (distance = 1)
		bool _possible = false;
		{
			auto np = trynext(_pos, 1, 1, _possible);
			if (_possible)
			{
				bb.set(np);
			};
			np = trynext(_pos, -1, 1, _possible);
			if (_possible)
			{
				bb.set(np);
			};
			np = trynext(_pos, 1, -1, _possible);
			if (_possible)
			{
				bb.set(np);
			};
			np = trynext(_pos, -1, -1, _possible);
			if (_possible)
			{
				bb.set(np);
			};
		};

		// Knights
		bb |= get_knight_attack_squares(_pos);

		// Diagonals
		bb |= make_diagonal_bits(_pos);
		return bb;
	};
	consteval auto calculate_threat_positions()
	{
		std::array<BitBoardCX, 64> bbs{};
		for (auto& v : positions_v)
		{
			bbs[static_cast<size_t>(v)] = calculate_threat_positions(v);
		}
		return bbs;
	};

	constexpr inline auto threat_positions_v = calculate_threat_positions();

	constexpr auto get_threat_positions(Position _pos)
	{
		return threat_positions_v[static_cast<size_t>(_pos)];
	};

	




	bool is_check(const chess::Board& _board, const chess::Color _forPlayer)
	{
		using namespace chess;

		const auto& p = _board.get_king(_forPlayer);

		if (p)
		{
			// Quick check that an enemy pieces is in one of the threat positions
			auto _threatPositions = BitBoard(get_threat_positions(p.position()));

			// Grab the friendly positions
			//const auto _friendlyPositions = (_forPlayer == Color::white) ?
			//	_board.get_white_piece_bitboard() :
			//	_board.get_black_piece_bitboard();

			// Grab the enemy positions
			//const auto _enemyPositions = (_forPlayer == Color::black) ?
			//	_board.get_white_piece_bitboard() :
			//	_board.get_black_piece_bitboard();



			// Remove the enemy king as it will never be a threat
			_threatPositions.reset(_board.get_king(!_forPlayer).position());

			if (_forPlayer == Color::white)
			{
				// If a piece is directly above us, remove the whole file above the king
				if (p.rank() != Rank::r8)
				{
					if (_board.has_friendy_piece(next(p.position(), 0, 1), _forPlayer))
					{
						_threatPositions &= ~make_file_bits(p.file(), p.rank(), Rank::r8);
					};
				};

				auto _threats = _threatPositions & _board.get_black_piece_bitboard();
				if (_threats.none())
				{
					// No threats, cannot be in check
					//std::cout << "Prevented check check\n";
					return false;
				};
			}
			else
			{
				// If a piece is directly below us, remove the whole file below the king
				if (p.rank() != Rank::r1 && _board.has_friendy_piece(next(p.position(), 0, -1), _forPlayer))
				{
					_threatPositions &= ~make_file_bits(p.file(), Rank::r1, p.rank());
				};
				_board.pieces_on_file(p.file());

				auto _threats = _threatPositions & _board.get_white_piece_bitboard();
				if (_threats.none())
				{
					// No threats, cannot be in check
					//std::cout << "Prevented check check\n";
					return false;
				};
			};

			return is_piece_attacked(_board, p);
		};
		return true;
	};
	bool is_checkmate(const chess::Board& _board, const chess::Color _forPlayer)
	{
		using namespace chess;

		if (!is_check(_board, _forPlayer))
		{
			return false;
		};

		static thread_local auto _bufferData = std::array<Move, 32>{};
		for (auto& v : _board.pieces())
		{
			if (v.color() == _forPlayer)
			{
				auto _buffer = MoveBuffer(_bufferData.data(), _bufferData.data() + _bufferData.size());
				const auto _bufferStart = _buffer.head();
				get_piece_moves(_board, v, _buffer, true);
				const auto _bufferEnd = _buffer.head();

				for (auto p = _bufferStart; p != _bufferEnd; ++p)
				{
					auto _futureBoard = _board;
					_futureBoard.move(*p);
					if (!is_check(_futureBoard, _forPlayer))
					{
						return false;
					};
				};
			};
		};
		
		return true;
	};



	constexpr inline chess::Rating material_value(const chess::PieceType& _piece)
	{
		switch (_piece)
		{
		case chess::PieceType::pawn:
			return 1.0f;
		case chess::PieceType::knight:
			return 2.0f;
		case chess::PieceType::bishop:
			return 2.0f;
		case chess::PieceType::rook:
			return 5.0f;
		case chess::PieceType::queen:
			return 10.0f;
		case chess::PieceType::king:
			return 1000.0f;
		};
	};


	constexpr inline auto white_distance_to_promote_v = std::array
	{
		7, // rank 1
		6, 5, 4, 3, 2, 1,
		0 // rank 8
	};
	constexpr inline auto black_distance_to_promote_v = std::array
	{
		0, // rank 1
		1, 2, 3, 4, 5, 6,
		7 // rank 8
	};

	constexpr inline auto make_pawn_promote_ratings(const auto& _distances)
	{
		std::array<Rating, 8> o{};
		auto oi = o.begin();
		for (auto it = _distances.rbegin(); it != _distances.rend(); ++it)
		{
			*oi = ((float)*it / 7.0f) * PAWN_PUSH_RATING;
			++oi;
		};
		return o;
	};

	template <typename T, size_t LN, size_t RN>
	constexpr inline auto concat_arrays(std::array<T, LN> lhs, std::array<T, RN> rhs)
	{
		auto o = std::array<T, LN + RN>{};
		auto i = o.begin();
		for (auto& v : lhs) { *(i++) = v; };
		for (auto& v : rhs) { *(i++) = v; };
		return o;
	};


	constexpr inline auto white_pawn_promote_rating_v = make_pawn_promote_ratings(white_distance_to_promote_v);
	constexpr inline auto black_pawn_promote_rating_v = make_pawn_promote_ratings(black_distance_to_promote_v);



	template <chess::Color Player>
	chess::Rating rate_board(const chess::Board& _board)
	{
		using namespace chess;

		constexpr auto& checkmate_rating_v = CHECKMATE_RATING;
		constexpr auto& blocked_queen_rating_v = BLOCKED_QUEEN_RATING;
		constexpr auto& blocked_rook_rating_v = BLOCKED_ROOK_RATING;
		constexpr auto& blocked_bishop_rating_v = BLOCKED_BISHOP_RATING;
		constexpr auto& pawn_push_rating_v = PAWN_PUSH_RATING;
		constexpr auto& castle_ability_rating_v = CASTLE_ABILITY_RATING;
		constexpr auto& development_rating_v = DEVELOPMENT_RATING;

		auto _rating = Rating(0);

		if (is_checkmate(_board, !Player))
		{
			return checkmate_rating_v;
		};

		if (_board.get_castle_kingside_flag(Player))
		{
			_rating += castle_ability_rating_v;
		};
		if (_board.get_castle_queenside_flag(Player))
		{
			_rating += castle_ability_rating_v;
		};
		if (_board.get_castle_kingside_flag(!Player))
		{
			_rating -= castle_ability_rating_v;
		};
		if (_board.get_castle_queenside_flag(!Player))
		{
			_rating -= castle_ability_rating_v;
		};

		const auto _pEnd = _board.pend();
		for (auto it = _board.pbegin(); it != _pEnd; ++it)
		{
			const auto& v = *it;
			switch (v)
			{
			case Piece::white_pawn:
			{
				if constexpr (Player == Color::white)
				{
					_rating += white_pawn_promote_rating_v[jc::to_underlying(v.rank())];
					_rating += material_value(Piece::pawn);
				}
				else
				{
					_rating -= white_pawn_promote_rating_v[jc::to_underlying(v.rank())];
					_rating -= material_value(Piece::pawn);
				};
			};
			break;
			case Piece::black_pawn:
			{
				if constexpr (Player == Color::white)
				{
					_rating -= black_pawn_promote_rating_v[jc::to_underlying(v.rank())];
					_rating -= material_value(Piece::pawn);
				}
				else
				{
					_rating += black_pawn_promote_rating_v[jc::to_underlying(v.rank())];
					_rating += material_value(Piece::pawn);
				};
			};
			break;

			case Piece::black_queen: [[fallthrough]];
			case Piece::black_bishop: [[fallthrough]];
			case Piece::black_rook:
			{
				if (v.rank() != Rank::r8)
				{
					if constexpr (Player == Color::white)
					{
						_rating -= development_rating_v;
					}
					else
					{
						_rating += development_rating_v;
					};
				};

				if constexpr (Player == Color::white)
				{
					_rating -= material_value(v.type());
				}
				else
				{
					_rating += material_value(v.type());
				};
			};
			break;

			case Piece::black_knight:
			{
				if constexpr (Player == Color::white)
				{
					_rating -= material_value(Piece::knight);
				}
				else
				{
					_rating += material_value(Piece::knight);
				};
			};
			break;

			case Piece::white_queen: [[fallthrough]];
			case Piece::white_bishop: [[fallthrough]];
			case Piece::white_rook:
			{
				if (v.rank() != Rank::r8)
				{
					if constexpr (Player == Color::white)
					{
						_rating += development_rating_v;
					}
					else
					{
						_rating -= development_rating_v;
					};
				};
				
				if constexpr (Player == Color::white)
				{
					_rating += material_value(v.type());
				}
				else
				{
					_rating -= material_value(v.type());
				};
			};
			break;

			case Piece::white_knight:
			{
				if constexpr (Player == Color::white)
				{
					_rating += material_value(Piece::knight);
				}
				else
				{
					_rating -= material_value(Piece::knight);
				};
			};
			break;

			case Piece::white_king:
			{
				if constexpr (Player == Color::white)
				{
					_rating += material_value(Piece::king);
				}
				else
				{
					_rating -= material_value(Piece::king);
				};
			};
			break; 
			case Piece::black_king:
			{
				if constexpr (Player == Color::white)
				{
					_rating -= material_value(Piece::king);
				}
				else
				{
					_rating += material_value(Piece::king);
				};
			};
			break;
			default:
				break;
			};
		};

		// 50 move rule is a draw
		if (_board.get_half_move_count() >= 50)
		{
			return -1000;
		};

		return _rating;
	};

	chess::Rating rate_board(const chess::Board& _board, chess::Color _forPlayer)
	{
		using namespace chess;
		if (_forPlayer == Color::white)
		{
			return rate_board<Color::white>(_board);
		}
		else
		{
			return rate_board<Color::black>(_board);
		};
	};

	chess::Rating rate_move(const chess::Board& _board, const chess::Move& _move, chess::Color _forPlayer, const bool _isCheck)
	{
		return rate_board(_board, _forPlayer);
	};

};