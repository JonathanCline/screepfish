#include "engine.hpp"

#include "chess/bitboard.hpp"

#include <array>
#include <vector>
#include <random>
#include <bitset>
#include <bit>
#include <iostream>
#include <chrono>

#include <jclib/functional.h>


namespace sch
{
	inline auto random_iter(auto& _range)
	{
		static std::random_device rnd{};
		static std::mt19937 mt{ rnd() };

		const auto _randomInt = mt();
		const auto _randomPct = (static_cast<double>(_randomInt) + static_cast<double>(mt.min())) /
			static_cast<double>(mt.max());

		const auto _rangeSize = std::ranges::distance(_range);
		const auto _randomIndex = static_cast<size_t>(ceil(static_cast<double>(_rangeSize) * _randomPct) - 1.0);

		if (_rangeSize == 0)
		{
			return std::ranges::end(_range);
		};

		return std::ranges::next(std::ranges::begin(_range), _randomIndex);
	};


	constexpr auto files = std::array{
		chess::File::a,
		chess::File::b,
		chess::File::c,
		chess::File::d,
		chess::File::e,
		chess::File::f,
		chess::File::g,
	};
	constexpr auto ranks = std::array{
		chess::Rank::r1,
		chess::Rank::r2,
		chess::Rank::r3,
		chess::Rank::r4,
		chess::Rank::r5,
		chess::Rank::r6,
		chess::Rank::r7,
	};


	struct MoveBuffer
	{
	public:

		chess::Move* head() const { return this->at_; };

		void write(chess::Move _move)
		{
			assert(this->at_ != this->end_);
			*this->at_ = std::move(_move);
			++this->at_;
		};
		void write(chess::Position _from, chess::Position _to)
		{
			return this->write(chess::Move(_from, _to));
		};

		MoveBuffer(chess::Move* _at, chess::Move* _end) :
			at_(_at), end_(_end)
		{
			assert(_at && _end && _at <= _end);
		};

	private:
		chess::Move* at_;
		chess::Move* end_;
	};


	struct PieceMovementCX
	{
		chess::BitBoardCX move;
		chess::BitBoardCX capture;
		chess::BoardPiece piece;

		constexpr PieceMovementCX() = default;
		constexpr PieceMovementCX(chess::BoardPiece _piece) :
			piece(_piece)
		{};
	};

	constexpr PieceMovementCX generate_move_bitboard_pawn(chess::BoardPiece _piece)
	{
		using namespace chess;

		const auto _position = _piece.position();
		const auto _file = _position.file();
		const auto _rank = _position.rank();

		// Change in rank, depends on piece color
		int _deltaRank = 1;

		// 2X delta rank for initial pawn move
		bool _doubleDeltaRank = false;

		if (_piece.color() == Color::white)
		{
			// Can move x2 on initial move if not blocked
			if (_rank == Rank::r2)
			{
				_doubleDeltaRank = true;
			};
			_deltaRank = 1;
		}
		else
		{
			// Can move x2 on initial move
			if (_rank == Rank::r7)
			{
				_doubleDeltaRank = true;
			};
			_deltaRank = -1;
		};

		// Bit board to write move positions into
		auto _moveBitboard = BitBoardCX();

		// Can move x2
		if (_doubleDeltaRank)
		{
			bool _possible = false;
			const auto _newPos = (_file, trynext(_rank, _deltaRank * 2, _possible));
			const auto _newPosMiddle = (_file, trynext(_rank, _deltaRank, _possible));
			if (_possible)
			{
				_moveBitboard.set(_newPos.file(), _newPos.rank());
			};
		}
		
		// Can move x1
		{
			bool _possible = false;
			const auto _newPos = (_file, trynext(_rank, _deltaRank, _possible));
			if (_possible)
			{
				_moveBitboard.set(_newPos.file(), _newPos.rank());
			};
		};

		auto _captureBitboard = chess::BitBoardCX();

		// Can capture diagonally
		{
			bool _possible = false;
			const auto _newPos = trynext(_position, 1, _deltaRank, _possible);
			if (_possible)
			{
				_captureBitboard.set(_newPos.file(), _newPos.rank());
			};
		};
		{
			bool _possible = false;
			const auto _newPos = trynext(_position, -1, _deltaRank, _possible);
			if (_possible)
			{
				// Check for piece to capture
				_captureBitboard.set(_newPos.file(), _newPos.rank());
			};
		};

		auto o = PieceMovementCX(_piece);
		o.capture = _captureBitboard;
		o.move = _moveBitboard;
		return o;
	};

	consteval auto generate_move_bitboards_pawn(chess::Color _color)
	{
		using namespace chess;
		auto _boards = std::array<PieceMovementCX, 48>{};
		auto it = _boards.begin();

		for (File f = File::a; f != File::h; f += 1)
		{
			for (Rank r = Rank::r2; r != Rank::r8; r += 1)
			{
				*(it++) = generate_move_bitboard_pawn(BoardPiece(PieceType::pawn, _color, (f, r)));
			};
		};

		return _boards;
	};

	constexpr static auto white_pawn_move_bitboards_v = generate_move_bitboards_pawn(chess::Color::white);
	constexpr static auto black_pawn_move_bitboards_v = generate_move_bitboards_pawn(chess::Color::black);

	inline auto* find_pawn_movement_bitboards(chess::Position _position, chess::Color _color)
	{
		using namespace chess;
		auto& _boards = ((_color == Color::white) ? white_pawn_move_bitboards_v : black_pawn_move_bitboards_v);
		for (auto& v : _boards)
		{
			if (v.piece.color() == _color && v.piece.position() == _position)
			{
				return &v;
			};
		};
		return (const sch::PieceMovementCX*)(nullptr);
	};



	struct PieceBitboards
	{
		chess::BitBoard friendly;
		chess::BitBoard enemy;
		chess::BitBoard all;
	};

	inline PieceBitboards get_piece_bitboards(const chess::Board& _board, chess::Color _myColor)
	{
		using namespace chess;

		auto _friendlyPieceBits = BitBoard();
		auto _enemyPieceBits = BitBoard();

		for (auto& v : _board.pieces())
		{
			if (v.color() == _myColor)
			{
				_friendlyPieceBits.set(v.file(), v.rank());
			}
			else
			{
				_enemyPieceBits.set(v.file(), v.rank());
			};
		};

		return { _friendlyPieceBits, _enemyPieceBits, _friendlyPieceBits | _enemyPieceBits };
	};





	inline auto get_pawn_moves(const chess::Board& _board, const chess::BoardPiece& _piece,
		MoveBuffer& _buffer)
	{
		using namespace chess;
		
		const auto _position = _piece.position();
		const auto _rank = _position.rank();
		const auto _file = _position.file();

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

		// Can move x2 if not blocked
		if (_doubleDeltaRank)
		{
			bool _possible = false;
			const auto _newPos = (_file, trynext(_rank, _deltaRank * 2, _possible));
			const auto _newPosMiddle = (_file, trynext(_rank, _deltaRank, _possible));
			if (_possible && !_board.get(_newPos) && !_board.get(_newPosMiddle))
			{
				_buffer.write(Move(_position, _newPos));
			};
		};

		// Can move x1 if not blocked
		{
			bool _possible = false;
			const auto _newPos = (_file, trynext(_rank, _deltaRank, _possible));
			if (_possible && !_board.get(_newPos))
			{
				_buffer.write(Move(_position, _newPos));
			};
		};

		// Can capture diagonally


		{
			bool _possible = false;
			const auto _newPos = trynext(_position, 1, _deltaRank, _possible);
			if (_possible)
			{
				// Check for piece to capture
				const auto _destPiece = _board.get(_newPos);
				if (_board.has_enemy_piece(_newPos, _piece.color()))
				{
					_buffer.write(Move(_position, _newPos));
				};
			};
		};
		{
			bool _possible = false;
			const auto _newPos = trynext(_position, -1, _deltaRank, _possible);
			if (_possible)
			{
				// Check for piece to capture
				if (_board.has_enemy_piece(_newPos, _piece.color()))
				{
					_buffer.write(Move(_position, _newPos));
				};
			};
		};
		
		// Add enpassant if flag set, this will never be set for our move
		if (_board.has_enpassant_target())
		{
			bool _possible = false;
			const auto _enpassantTarget = _board.enpassant_target();
			if (_enpassantTarget == trynext(_position, -1, _deltaRank, _possible) ||
				_enpassantTarget == trynext(_position, 1, _deltaRank, _possible))
			{
				_buffer.write(Move(_position, _enpassantTarget));
			};
		};
	};
	inline auto get_rook_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer)
	{
		using namespace chess;

		const auto _position = _piece.position();
		const auto _rank = _position.rank();
		const auto _file = _position.file();

		auto _minRank = Rank::r1;
		auto _maxRank = Rank::r8;
		auto _minFile = File::a;
		auto _maxFile = File::h;

		for (Rank r = _minRank; r != _rank; r += 1)
		{
			const auto p = _board.get((_file, r));
			if (p)
			{
				if (p.color() == _piece.color())
				{
					_minRank = r + 1;
				}
				else
				{
					_minRank = r;
				};
			};
		};
		for (Rank r = _rank + 1; r <= Rank::r8; r += 1)
		{
			const auto p = _board.get((_file, r));
			if (p)
			{
				if (p.color() == _piece.color())
				{
					_maxRank = r - 1;
					break;
				}
				else
				{
					_maxRank = r;
					break;
				};
			};
		};
		for (File f = _minFile; f != _file; f += 1)
		{
			const auto p = _board.get((f, _rank));
			if (p)
			{
				if (p.color() == _piece.color())
				{
					_minFile = f + 1;
				}
				else
				{
					_minFile = f;
				};
			};
		};
		for (File f = _file + 1; f <= File::h; f += 1)
		{
			const auto p = _board.get((f, _rank));
			if (p)
			{
				if (p.color() == _piece.color())
				{
					_maxFile = f - 1;
					break;
				}
				else
				{
					_maxFile = f;
					break;
				};
			};
		};

		for (Rank r = _minRank; r <= _maxRank; r += 1)
		{
			if (r == _rank) continue;
			_buffer.write(_position, Position(_file, r));
		};
		for (File f = _minFile; f <= _maxFile; f += 1)
		{
			if (f == _file) continue;
			_buffer.write(_position, Position(f, _rank));
		};
	};
	inline auto get_knight_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer)
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
	inline auto get_king_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer)
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
	};
	inline auto get_bishop_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer)
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
			int n = 1;
			while (true)
			{
				auto [df, dr] = _direction;
				df *= n;
				dr *= n;

				_nextPosition = trynext(_position, df, dr, _possible);
				if (_possible)
				{
					if (_board.has_friendy_piece(_nextPosition, _piece.color()))
					{
						break;
					}
					else
					{
						_buffer.write(_position, _nextPosition);
						if (_board.has_enemy_piece(_nextPosition, _piece.color()))
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
	};
	inline auto get_queen_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer)
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
			int n = 1;
			while (true)
			{
				auto [df, dr] = _direction;
				df *= n;
				dr *= n;

				_nextPosition = trynext(_position, df, dr, _possible);
				if (_possible)
				{
					if (_board.has_friendy_piece(_nextPosition, _piece.color()))
					{
						break;
					}
					else
					{
						_buffer.write(_position, _nextPosition);
						if (_board.has_enemy_piece(_nextPosition, _piece.color()))
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
	};

	inline auto get_piece_moves(const chess::Board& _board, const chess::BoardPiece& _piece,
		MoveBuffer& _buffer)
	{
		using namespace chess;

		const auto _piecePos = _piece.position();
		switch (_piece.type())
		{
		case PieceType::pawn:
		{
			get_pawn_moves(_board, _piece, _buffer);
		};
		break;
		case PieceType::rook:
		{
			get_rook_moves(_board, _piece, _buffer);
		};
		break;
		case PieceType::knight:
		{
			get_knight_moves(_board, _piece, _buffer);
		};
		break;
		case PieceType::king:
		{
			get_king_moves(_board, _piece, _buffer);
		};
		break;
		case PieceType::bishop:
		{
			get_bishop_moves(_board, _piece, _buffer);
		};
		break;
		case PieceType::queen:
		{
			get_queen_moves(_board, _piece, _buffer);
		};
		break;
		};
	};


	inline bool is_piece_attacked(const chess::Board& _board, const chess::BoardPiece& _piece)
	{
		static thread_local auto _bufferData = std::array<chess::Move, 256>{};
		auto _buffer = MoveBuffer(_bufferData.data(), _bufferData.data() + _bufferData.size());

		for (auto& p : _board.pieces())
		{
			if (p.color() != _piece.color())
			{
				auto _movesStart = _buffer.head();
				get_piece_moves(_board, p, _buffer);
				auto _movesEnd = _buffer.head();
				
				for (auto mp = _movesStart; mp != _movesEnd; ++mp)
				{
					if (mp->to() == _piece.position())
					{
						return true;
					};
				};
			};
		};
		return false;
	};

	inline bool is_check(const chess::Board& _board, const chess::Color _forPlayer)
	{
		using namespace chess;
		const auto _kingIt = _board.find(chess::Piece(chess::PieceType::king, _forPlayer));
		return is_piece_attacked(_board, *_kingIt);
	};


	inline auto get_moves(const chess::Board& _board, const chess::Color _forPlayer)
	{
		static thread_local auto _bufferData = std::array<chess::Move, 256>{};
		auto _buffer = MoveBuffer(_bufferData.data(), _bufferData.data() + _bufferData.size());
		
		using namespace chess;
		
		const auto _bufferStart = _buffer.head();
		for (auto& _piece : _board.pieces())
		{
			if (_piece.color() == _forPlayer)
			{
				get_piece_moves(_board, _piece, _buffer);
			};
		};
		const auto _bufferEnd = _buffer.head();

		auto _moves = std::vector<Move>(_bufferData.size(), Move());
		auto it = _moves.begin();
		for (auto p = _bufferStart; p != _bufferEnd; ++p)
		{
			auto _futureBoard = _board;
			_futureBoard.move_piece(p->from(), p->to(), p->promotion());
			if (!is_check(_futureBoard, _forPlayer))
			{
				*it = *p;
				++it;
			};
		};

		_moves.erase(it, _moves.end());
		return _moves;
	};

	inline bool is_checkmate(const chess::Board& _board, const chess::Color _forPlayer)
	{
		using namespace chess;
		
		if (!is_check(_board, _forPlayer))
		{
			return false;
		};

		auto _possibleMoves = get_moves(_board, _forPlayer);
		for (auto& v : _possibleMoves)
		{
			auto _futureBoard = _board;
			_futureBoard.move(v);
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

	inline chess::Rating rate_board(const chess::Board& _board, chess::Color _forPlayer)
	{
		using namespace chess;
		auto _rating = Rating(0);

		if (is_checkmate(_board, _forPlayer))
		{
			_rating -= 100000;
			return _rating;
		}
		else if (is_checkmate(_board, !_forPlayer))
		{
			_rating += 100000;
			return _rating;
		};

		for (auto& v : _board.pieces())
		{
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
	inline chess::Rating rate_move(const chess::Board& _board, const chess::Move& _move, chess::Color _forPlayer)
	{
		auto _futureBoard = _board;
		_futureBoard.move(_move);
		auto _rating = rate_board(_futureBoard, _forPlayer);
		if (_rating > 10000)
		{
			std::cout << "Found checkmate " << _move << '\n';
		};
		return _rating;
	};

	struct RatedMove
	{
		constexpr auto operator<=>(const RatedMove& rhs) const
		{
			return this->rating <=> rhs.rating;
		};
		chess::Move move;
		chess::Rating rating;
	};

	inline RatedMove best_move_search(const chess::Board& _board, chess::Color _forPlayer, int _depth2)
	{
		using namespace chess;

		std::vector<RatedMove> _ratedMoves{};

		{
			auto _moves = get_moves(_board, _forPlayer);
			_ratedMoves.resize(_moves.size());
			auto o = _ratedMoves.begin();
			for (auto& v : _moves)
			{
				auto _futureBoard = _board;
				_futureBoard.move(v);
				
				auto _rating = rate_board(_futureBoard, _forPlayer);
				if (_rating < 10000 && _rating > -10000 && _depth2 != 0)
				{
					const auto _bestResponse = best_move_search(_futureBoard, !_forPlayer, _depth2 - 1);
					_rating = -_bestResponse.rating;
				};

				*o = RatedMove{ v, _rating };
				++o;
			};
		};
		std::ranges::sort(_ratedMoves, jc::greater);

		if (_ratedMoves.empty())
		{
			return RatedMove{};
		}
		else
		{
			return _ratedMoves.front();
		};
	};
	inline chess::Move best_move(const chess::Board& _board, chess::Color _forPlayer, int _depth)
	{
		const auto _rated = best_move_search(_board, _forPlayer, _depth * 2);
		return _rated.move;
	};



	chess::Move ScreepFish::play_turn(chess::IGame& _game)
	{
		using namespace chess;

		const auto& _board = _game.board();
		const auto& _myColor = this->my_color_;
		
		//std::chrono::steady_clock;
		const auto _move = best_move(_board, _myColor, 2);
		return _move;
	};
	





	void ScreepFish::set_color(chess::Color _color)
	{
		this->my_color_ = _color;
	};
	void ScreepFish::set_board(chess::Board _board)
	{
		this->board_ = std::move(_board);
	};

};