#include "move.hpp"

#include <iostream>

namespace chess
{

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



	bool is_piece_attacked_by_pawn(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
	{
		using namespace chess;

		const auto _file = _piece.file();
		const auto _rank = _piece.rank();
		const auto _fromFile = _byPiece.file();
		const auto _fromRank = _byPiece.rank();

		if (_file == _fromFile + 1 || _file == _fromFile - 1)
		{
			if (_byPiece.color() == Color::white)
			{
				return _rank == _fromRank + 1;
			}
			else
			{
				return _rank == _fromRank - 1;
			};
		};

		return false;
	};
	bool is_piece_attacked_by_knight(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
	{
		using namespace chess;

		const auto _position = _byPiece.position();
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
			if (_possible && _nextPosition == _piece.position())
			{
				return true;
			};
		};

		return false;
	};
	bool is_piece_attacked_by_bishop(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
	{
		using namespace chess;

		const auto _position = _byPiece.position();
		constexpr auto _directionPairs = std::array
		{
			std::pair{  1,  1 },
			std::pair{ -1,  1 },
			std::pair{  1, -1 },
			std::pair{ -1, -1 },
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
	bool is_piece_attacked_by_rook(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
	{
		using namespace chess;

		const auto _position = _byPiece.position();
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

					if (_board.has_friendy_piece(_nextPosition, _byPiece.color()))
					{
						break;
					}
					else
					{
						if (_board.has_enemy_piece(_nextPosition, _byPiece.color()))
						{
							break;
						};
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


	bool is_piece_attacked(const chess::Board& _board, const chess::BoardPiece& _piece)
	{
		using namespace chess;

		const auto _end = _board.pend();
		for (auto it = _board.pbegin(); it != _end; ++it)
		{
			auto& p = *it;
			if (p.color() != _piece.color())
			{
				switch (p.type())
				{
				case PieceType::pawn:
					if (is_piece_attacked_by_pawn(_board, _piece, p))
					{
						return true;
					};
					break;
				case PieceType::knight:
					if (is_piece_attacked_by_knight(_board, _piece, p))
					{
						return true;
					};
					break;
				case PieceType::bishop:
					if (is_piece_attacked_by_bishop(_board, _piece, p))
					{
						return true;
					};
					break;
				case PieceType::rook:
					if (is_piece_attacked_by_rook(_board, _piece, p))
					{
						return true;
					};
					break;
				case PieceType::queen:
					if (is_piece_attacked_by_queen(_board, _piece, p))
					{
						return true;
					};
					break;
				case PieceType::king:
					if (is_piece_attacked_by_king(_board, _piece, p))
					{
						return true;
					};
					break;
				default:
					break;
				};
			};
		};
		return false;
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
			const auto _newPosOne = (_file, trynext(_rank, _deltaRank, _possible));

			if (_possible && _board.is_empty(_newPosOne))
			{
				_buffer.write(Move(_position, _newPosOne,
					(_newPosOne.rank() == Rank::r8 || _newPosOne.rank() == Rank::r1) ?
					PieceType::queen :
					PieceType::none
				));

				// Can move x2 if not blocked
				if (_doubleDeltaRank)
				{
					const auto _newPosTwo = (_file, trynext(_rank, _deltaRank * 2, _possible));
					if (_possible && _board.is_empty(_newPosTwo))
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
				_buffer.write(Move(_position, _newPos));
			};
		};
		if (_file != File::a)
		{
			// Check for piece to capture
			const auto _newPos = next(_position, -1, _deltaRank);
			if ((_board.has_enpassant_target() && _board.enpassant_target() == _newPos)
				|| _board.has_enemy_piece(_newPos, _piece.color()))
			{
				_buffer.write(Move(_position, _newPos));
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

		if (_board.get_castle_kingside_flag(_piece.color()))
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

			if (_board.is_empty((File::f, _dest.rank()))
				&& _board.is_empty((File::g, _dest.rank())))
			{
				_buffer.write(_position, _dest);
			};
		}
		if (_board.get_castle_queenside_flag(_piece.color()))
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

			if (_board.is_empty((File::d, _dest.rank()))
				&& _board.is_empty((File::c, _dest.rank()))
				&& _board.is_empty((File::b, _dest.rank())))
			{
				_buffer.write(_position, _dest);
			};
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


	bool is_check(const chess::Board& _board, const chess::Color _forPlayer)
	{
		using namespace chess;
		auto p = _board.pfind(chess::PieceType::king, _forPlayer);
		if (p != _board.pend())
		{
			return is_piece_attacked(_board, *p);
		};
		return true;
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
	bool is_checkmate(const chess::Board& _board, const chess::Color _forPlayer)
	{
		using namespace chess;

		if (!is_check(_board, _forPlayer))
		{
			return false;
		};

		static thread_local auto _bufferData = std::array<Move, 256>{};
		auto _buffer = MoveBuffer(_bufferData.data(), _bufferData.data() + _bufferData.size());
		const auto _bufferStart = _buffer.head();
		get_moves(_board, _forPlayer, _buffer, true);
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
		return true;
	};



	inline chess::Rating material_value(const chess::PieceType& _piece)
	{
		switch (_piece)
		{
		case chess::PieceType::pawn:
			return 1;
		case chess::PieceType::knight:
			return 2;
		case chess::PieceType::bishop:
			return 2;
		case chess::PieceType::rook:
			return 5;
		case chess::PieceType::queen:
			return 10;
		case chess::PieceType::king:
			return 1000;
		};
	};


	chess::Rating rate_move(const chess::Board& _board, const chess::Move& _move, chess::Color _forPlayer, const bool _isCheck)
	{
		using namespace chess;

		auto _rating = Rating(0);

		if (is_checkmate(_board, !_forPlayer))
		{
			return 100000.0f;
		};

		if (_board.get(_move.from()) == Piece::king)
		{
			if (_board.get_castle_kingside_flag(_forPlayer))
			{
				_rating -= 0.00001f;
			};
			if (_board.get_castle_queenside_flag(_forPlayer))
			{
				_rating -= 0.00001f;
			};
		};

		for (auto& v : _board.pieces())
		{
			if (v == Piece::pawn)
			{
				int _closeToPromote = 0;
				if (v.color() == Color::white)
				{
					_closeToPromote =
						(int)v.rank();
				}
				else
				{
					_closeToPromote =
						-((int)v.rank() - (int)Rank::r8);
				};
				_rating += ((float)_closeToPromote / 1000.0f);				
			}

			if (v.color() == _forPlayer)
			{
				_rating += material_value(v.type());
			}
			else
			{
				_rating -= material_value(v.type());
			};
		};

		return _rating;
	};

};