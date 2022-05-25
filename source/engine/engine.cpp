#include "engine.hpp"

#include <array>
#include <vector>
#include <random>
#include <bitset>
#include <bit>

namespace chess
{
	namespace impl
	{
		template <typename IndexT>
		class BitArray
		{
		public:
			using index_type = IndexT;

			void set(index_type _idx, bool _value)
			{
				this->bits_.set(static_cast<size_t>(_idx), _value);
			};
			void set(index_type _idx)
			{
				this->bits_.set(static_cast<size_t>(_idx));
			};
			void reset(index_type _idx)
			{
				this->bits_.reset(static_cast<size_t>(_idx));
			};
			void reset()
			{
				this->bits_.reset();
			};

			bool test(index_type _idx) const
			{
				return this->bits_.test(static_cast<size_t>(_idx));
			};

			void invert()
			{
				this->bits_.flip();
			};

			bool all() const
			{
				return this->bits_.all();
			};
			bool any() const
			{
				return this->bits_.any();
			};
			bool none() const
			{
				return this->bits_.none();
			};

			constexpr BitArray() = default;
			constexpr BitArray(bool _value) :
				bits_((_value)? 0xFF : 0x00)
			{}

		private:
			std::bitset<8> bits_;
		};
	};

	using FileBits = impl::BitArray<File>;
	using RankBits = impl::BitArray<Rank>;

};


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


	



	inline auto get_pawn_moves(const chess::Board& _board, const chess::BoardPiece& _piece)
	{
		using namespace chess;
		
		const auto _position = _piece.position();
		const auto _rank = _position.rank();
		const auto _file = _position.file();

		auto _moves = std::vector<Position>();

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

		// Can move x2 if not blocked
		if (_doubleDeltaRank)
		{
			bool _possible = false;
			const auto _newPos = (_file, trynext(_rank, _deltaRank * 2, _possible));
			const auto _newPosMiddle = (_file, trynext(_rank, _deltaRank, _possible));
			if (_possible && !_board.get(_newPos) && !_board.get(_newPosMiddle))
			{
				_moves.push_back(_newPos);
			};
		};

		// Can move x1 if not blocked
		{
			bool _possible = false;
			const auto _newPos = (_file, trynext(_rank, _deltaRank, _possible));
			if (_possible && !_board.get(_newPos))
			{
				_moves.push_back(_newPos);
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
				if (_destPiece && _destPiece->color() != _piece.color())
				{
					_moves.push_back(_newPos);
				};
			};
		};
		{
			bool _possible = false;
			const auto _newPos = trynext(_position, -1, _deltaRank, _possible);
			if (_possible)
			{
				// Check for piece to capture
				const auto _destPiece = _board.get(_newPos);
				if (_destPiece && _destPiece->color() != _piece.color())
				{
					_moves.push_back(_newPos);
				};
			};
		};

		return _moves;
	};
	inline auto get_rook_moves(const chess::Board& _board, const chess::BoardPiece& _piece)
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
				if (p->color() == _piece.color())
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
				if (p->color() == _piece.color())
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
				if (p->color() == _piece.color())
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
				if (p->color() == _piece.color())
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

		auto _moves = std::vector<Position>();
		for (Rank r = _minRank; r <= _maxRank; r += 1)
		{
			if (r == _rank) continue;
			_moves.push_back(Position(_file, r));
		};
		for (File f = _minFile; f <= _maxFile; f += 1)
		{
			if (f == _file) continue;
			_moves.push_back(Position(f, _rank));
		};

		return _moves;
	};
	inline auto get_knight_moves(const chess::Board& _board, const chess::BoardPiece& _piece)
	{
		using namespace chess;
		auto _moves = std::vector<Position>();

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
				_moves.push_back(_nextPosition);
			};
		};

		return _moves;
	};
	inline auto get_king_moves(const chess::Board& _board, const chess::BoardPiece& _piece)
	{
		using namespace chess;
		auto _moves = std::vector<Position>();

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
				_moves.push_back(_nextPosition);
			};
		};

		return _moves;
	};
	inline auto get_bishop_moves(const chess::Board& _board, const chess::BoardPiece& _piece)
	{
		using namespace chess;
		auto _moves = std::vector<Position>();

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
						_moves.push_back(_nextPosition);
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

		return _moves;
	};
	inline auto get_queen_moves(const chess::Board& _board, const chess::BoardPiece& _piece)
	{
		using namespace chess;
		auto _moves = std::vector<Position>();

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
						_moves.push_back(_nextPosition);
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

		return _moves;
	};

	inline auto get_piece_moves(const chess::Board& _board, const chess::BoardPiece& _piece)
	{
		using namespace chess;
		auto _moves = std::vector<chess::Move>();
		switch (_piece.type())
		{
		case PieceType::pawn:
		{
			const auto _piecePos = _piece.position();
			auto _dests = get_pawn_moves(_board, _piece);
			for (auto& v : _dests)
			{
				_moves.push_back(Move(_piecePos, v));
			};
		};
		break;
		case PieceType::rook:
		{
			const auto _piecePos = _piece.position();
			auto _dests = get_rook_moves(_board, _piece);
			for (auto& v : _dests)
			{
				_moves.push_back(Move(_piecePos, v));
			};
		};
		break;
		case PieceType::knight:
		{
			const auto _piecePos = _piece.position();
			auto _dests = get_knight_moves(_board, _piece);
			for (auto& v : _dests)
			{
				_moves.push_back(Move(_piecePos, v));
			};
		};
		break;
		case PieceType::king:
		{
			const auto _piecePos = _piece.position();
			auto _dests = get_king_moves(_board, _piece);
			for (auto& v : _dests)
			{
				_moves.push_back(Move(_piecePos, v));
			};
		};
		break;
		case PieceType::bishop:
		{
			const auto _piecePos = _piece.position();
			auto _dests = get_bishop_moves(_board, _piece);
			for (auto& v : _dests)
			{
				_moves.push_back(Move(_piecePos, v));
			};
		};
		break;
		case PieceType::queen:
		{
			const auto _piecePos = _piece.position();
			auto _dests = get_queen_moves(_board, _piece);
			for (auto& v : _dests)
			{
				_moves.push_back(Move(_piecePos, v));
			};
		};
		break;
		};
		return _moves;
	};


	inline bool is_piece_attacked(const chess::Board& _board, const chess::BoardPiece& _piece)
	{
		for (auto& p : _board.pieces())
		{
			if (p.color() != _piece.color())
			{
				const auto _moves = get_piece_moves(_board, p);
				for (auto& v : _moves)
				{
					if (v.to() == _piece.position())
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
		using namespace chess;
		auto _moves = std::vector<chess::Move>();
		auto _myPieces = std::vector<BoardPiece>();
		for (auto& _piece : _board.pieces())
		{
			if (_piece.color() == _forPlayer)
			{
				auto _pieceMoves = get_piece_moves(_board, _piece);
				_moves.insert(_moves.end(), _pieceMoves.begin(), _pieceMoves.end());
			};
		};

		for (auto it = _moves.begin(); it != _moves.end();)
		{
			auto _futureBoard = _board;
			_futureBoard.move_piece(it->from(), it->to(), it->promotion());
			if (is_check(_futureBoard, _forPlayer))
			{
				it = _moves.erase(it);
			}
			else
			{
				++it;
			};
		};

		return _moves;
	};

	inline bool is_checkmate(const chess::Board& _board, const chess::Color _forPlayer)
	{
		using namespace chess;
		const auto _kingIt = _board.find(chess::Piece(chess::PieceType::king, _forPlayer));
		return is_piece_attacked(_board, *_kingIt);
	};




	chess::Move ScreepFish::play_turn(chess::IGame& _game)
	{
		using namespace chess;

		const auto& _board = _game.board();
		auto _moves = get_moves(_board, this->my_color_);
		
		for (auto& v : _moves)
		{
			if (_board.get(v.to()))
			{
				return v;
			};
		};
		
		return *random_iter(_moves);
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