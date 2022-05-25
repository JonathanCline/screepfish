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


	bool ScreepFish::is_piece_attacked_by_pawn(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
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
	bool ScreepFish::is_piece_attacked_by_knight(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
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
	bool ScreepFish::is_piece_attacked_by_bishop(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
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
	bool ScreepFish::is_piece_attacked_by_rook(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
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
	bool ScreepFish::is_piece_attacked_by_queen(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece)
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



	bool ScreepFish::is_piece_attacked(const chess::Board& _board, const chess::BoardPiece& _piece)
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
					if (this->is_piece_attacked_by_pawn(_board, _piece, p))
					{
						return true;
					};
					break;
				case PieceType::knight:
					if (this->is_piece_attacked_by_knight(_board, _piece, p))
					{
						return true;
					};
					break;
				case PieceType::bishop:
					if (this->is_piece_attacked_by_bishop(_board, _piece, p))
					{
						return true;
					};
					break;
				case PieceType::rook:
					if (this->is_piece_attacked_by_rook(_board, _piece, p))
					{
						return true;
					};
					break;
				case PieceType::queen:
					if (this->is_piece_attacked_by_queen(_board, _piece, p))
					{
						return true;
					};
					break;
				case PieceType::king:
					if (_piece.type() == PieceType::king)
					{
						return false;
					};
					[[fallthrough]];
				default:
					break;
				};
			};
		};
		return false;
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



	void ScreepFish::get_pawn_moves(const chess::Board& _board, const chess::BoardPiece& _piece,
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
					(_newPosOne.rank() == Rank::r8 || _newPosOne.rank() == Rank::r1)? 
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
	void ScreepFish::get_rook_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck)
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
	void ScreepFish::get_knight_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck)
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
	void ScreepFish::get_king_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck)
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
	void ScreepFish::get_bishop_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck)
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
	void ScreepFish::get_queen_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck)
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

	void ScreepFish::get_piece_moves(const chess::Board& _board, const chess::BoardPiece& _piece,
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


	bool ScreepFish::is_check(const chess::Board& _board, const chess::Color _forPlayer)
	{
		using namespace chess;
		auto p = _board.pfind(chess::PieceType::king, _forPlayer);
		if (p != _board.pend())
		{
			return is_piece_attacked(_board, *p);
		};
		return true;
	};


	void ScreepFish::get_moves(const chess::Board& _board, const chess::Color _forPlayer, MoveBuffer& _buffer, const bool _isCheck)
	{
		static thread_local auto _bufferData = std::array<chess::Move, 256>{};
		auto _movesBuffer = MoveBuffer(_bufferData.data(), _bufferData.data() + _bufferData.size());

		using namespace chess;

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

	bool ScreepFish::is_checkmate(const chess::Board& _board, const chess::Color _forPlayer)
	{
		using namespace chess;

		//auto it = this->get_cached(hash(_board, false));
		//if (it)
		//{
		//	return (_forPlayer == chess::Color::white) ? it->is_wcheckmate : it->is_bcheckmate;
		//};


		if (!is_check(_board, _forPlayer))
		{
			return false;
		};

		static thread_local auto _bufferData = std::array<Move, 256>{};
		auto _buffer = MoveBuffer(_bufferData.data(), _bufferData.data() + _bufferData.size());
		const auto _bufferStart = _buffer.head();
		this->get_moves(_board, _forPlayer, _buffer, true);
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

	chess::Rating ScreepFish::rate_move(const chess::Board& _board, const chess::Move& _move, chess::Color _forPlayer)
	{
		using namespace chess;

		auto _rating = Rating(0);
		if (is_checkmate(_board, !_forPlayer))
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

	RatedMove ScreepFish::best_move_search(const chess::Board& _board, chess::Color _forPlayer, int _depth2)
	{
		using namespace chess;

		std::vector<RatedMove> _ratedMoves{};

		{
			auto _bufferData = std::array<Move, 256>{};
			auto _buffer = MoveBuffer(_bufferData.data(), _bufferData.data() + _bufferData.size());
			const auto _bufferStart = _buffer.head();
			this->get_moves(_board, _forPlayer, _buffer, false);
			const auto _bufferEnd = _buffer.head();
			
			_ratedMoves.resize(_bufferEnd - _bufferStart);
			auto o = _ratedMoves.begin();
			for (auto p = _bufferStart; p != _bufferEnd; ++p)
			{
				auto _futureBoard = _board;
				_futureBoard.move(*p);

				// Calculation time
				auto _rating = this->rate_move(_futureBoard, *p, _forPlayer);
				if (_rating < 10000 && _rating > -10000 && _depth2 != 0)
				{
					const auto _bestResponse = best_move_search(_futureBoard, !_forPlayer, _depth2 - 1);
					_rating = -_bestResponse.rating;
				};

				*o = RatedMove{ *p, _rating };
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
			auto _bestRating = _ratedMoves.front().rating;
			for (auto it = _ratedMoves.begin(); it != _ratedMoves.end(); ++it)
			{
				if (it->rating != _bestRating)
				{
					_ratedMoves.erase(it, _ratedMoves.end());
					break;
				};
			};

			return *random_iter(_ratedMoves);
		};
	};
	chess::Move ScreepFish::best_move(const chess::Board& _board, chess::Color _forPlayer, int _depth)
	{
		const auto _rated = best_move_search(_board, _forPlayer, _depth);
		return _rated.move;
	};

	void ScreepFish::cache(const chess::Board& _board)
	{
	};
	const ScreepFish::BoardCache* ScreepFish::get_cached(size_t _hash)
	{
		return nullptr;
	};


	chess::Move ScreepFish::play_turn(chess::IGame& _game)
	{
		using namespace chess;

		const auto& _board = _game.board();
		const auto& _myColor = this->my_color_;
		
		const auto _clock = std::chrono::steady_clock{};
		const auto t0 = _clock.now();

		const auto _move = best_move(_board, _myColor, 4);
		
		const auto t1 = _clock.now();
		const auto td = t1 - t0;
		std::cout << "Delta time : " << std::chrono::duration_cast<std::chrono::duration<double>>(td) << '\n';

		auto _newBoard = _board;
		_newBoard.move(_move);

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