#pragma once

/** @file */

#include "position.hpp"

#include "utility/number.hpp"
#include "utility/utility.hpp"

#include <jclib/type_traits.h>

#include <iosfwd>
#include <string>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>
#include <compare>
#include <ranges>
#include <algorithm>
#include <optional>
#include <array>


namespace chess
{



	/**
	 * @brief Named piece / square colors.
	*/
	enum class Color : bool
	{
		black,
		white
	};


	/**
	 * @brief Inverts the given color.
	 * 
	 * white -> black
	 * black -> white
	 * 
	 * @param rhs Color to invert.
	 * @return Inverted (opposite) color.
	*/
	constexpr Color operator!(Color rhs) noexcept
	{
		return Color(!jc::to_underlying(rhs));
	};







	enum class OutcomeType
	{
		none,
		draw,
		mate,
	};

	using Rating = int_fast32_t;

	












	/**
	 * @brief Represents a movement of a piece by a player.
	*/
	class PieceMove
	{
	public:

		/**
		 * @brief Gets the position the piece moved from.
		*/
		constexpr Position from() const noexcept { return this->from_; };

		/**
		 * @brief Gets the position the piece moved to.
		*/
		constexpr Position to() const noexcept { return this->to_; };



		constexpr PieceMove() = default;
		constexpr PieceMove(Position _from, Position _to) :
			from_(_from), to_(_to)
		{};

	private:

		/**
		 * @brief The position the piece moved from.
		*/
		Position from_;

		/**
		 * @brief The position the piece moved to.
		*/
		Position to_;
	};

	/**
	 * @brief Parses a move value from a string.
	 * 
	 * Format is assumed like "a1b2" where 'a1' is from pos and 'b2' is to pos.
	 *
	 * @param _str String to parse.
	 * @param _value Where to write the parsed value.
	 * @return String with parsed characters removed.
	*/
	constexpr std::string_view fromstr(std::string_view _str, PieceMove& _value)
	{
		// Parse individual positions.
		Position _from, _to;
		_str = fromstr(_str, _from);
		_str = fromstr(_str, _to);

		// Write parsed move.
		_value = PieceMove(_from, _to);
		return _str;
	};

	std::ostream& operator<<(std::ostream& _ostr, const PieceMove& _value);



	/**
	 * @brief Types of chess pieces.
	*/
	enum class PieceType : uint8_t
	{
		none = 0, // maybe
		pawn = 1,
		knight = 2,
		bishop = 3,
		rook = 4,
		queen = 5,
		king = 6
	};


	class Move : public PieceMove
	{
	public:

		constexpr PieceType promotion() const
		{
			return this->promotion_;
		};

		constexpr Move() = default;
		constexpr Move(Position _from, Position _to, PieceType _promotion) :
			PieceMove(_from, _to),
			promotion_{ _promotion }
		{};
		constexpr Move(Position _from, Position _to) :
			Move(_from, _to, PieceType{})
		{};
	
	private:
		PieceType promotion_;
	};

	/**
	 * @brief Parses a move value from a string.
	 *
	 * Format is assumed like "a1b2" where 'a1' is from pos and 'b2' is to pos.
	 *
	 * @param _str String to parse.
	 * @param _value Where to write the parsed value.
	 * @return String with parsed characters removed.
	*/
	constexpr std::string_view fromstr(std::string_view _str, Move& _value)
	{
		// Parse individual positions.
		Position _from, _to;
		_str = fromstr(_str, _from);
		_str = fromstr(_str, _to);

		auto _promotion = PieceType::none;
		if (!_str.empty())
		{
			const auto c = _str.front();
			switch(c)
			{
			case 'q':
				_promotion = PieceType::queen;
				break;
			case 'n':
				_promotion = PieceType::knight;
				break;
			case 'r':
				_promotion = PieceType::rook;
				break;
			case 'b':
				_promotion = PieceType::bishop;
				break;
			default:
				break;
			};
			if (_promotion != PieceType::none)
			{
				_str.remove_prefix(1);
			};
		};

		// Write parsed move.
		_value = Move(_from, _to, _promotion);
		return _str;
	};

	std::ostream& operator<<(std::ostream& _ostr, const Move& _value);







	/**
	 * @brief Represents a chess piece with defined color.
	*/
	class Piece
	{
	public:

		/**
		 * @brief Types of chess pieces.
		*/
		using Type = PieceType;

		/**
		 * @brief Gets the type of the piece.
		 * @return Chess piece type.
		*/
		constexpr Type type() const noexcept
		{
			return this->type_;
		};

		/**
		 * @brief Checks that the piece type is valid.
		 * @return True if valid, false otherwise.
		*/
		constexpr explicit operator bool() const noexcept
		{
			return this->type() != Type::none;
		};

		/**
		 * @brief Gets the color of the piece.
		 * @return Chess piece color.
		*/
		constexpr Color color() const noexcept
		{
			return this->color_;
		};


		// Compares color AND piece type
		constexpr bool operator==(const Piece& rhs) const noexcept = default;
		// Compares color AND piece type
		constexpr bool operator!=(const Piece& rhs) const noexcept = default;

		// Compares piece type
		friend inline constexpr bool operator==(const Piece& lhs, const PieceType& rhs) noexcept
		{
			return lhs.type() == rhs;
		};
		// Compares piece type
		friend inline constexpr bool operator==(const PieceType& lhs, const Piece& rhs) noexcept
		{
			return lhs == rhs.type();
		};

		// Compares piece type
		friend inline constexpr bool operator!=(const Piece& lhs, const PieceType& rhs) noexcept
		{
			return lhs.type() != rhs;
		};
		// Compares piece type
		friend inline constexpr bool operator!=(const PieceType& lhs, const Piece& rhs) noexcept
		{
			return lhs != rhs.type();
		};

		



		constexpr Piece() noexcept = default;
		constexpr Piece(Type _type, Color _color) noexcept :
			type_(_type), color_(_color)
		{};
		
		constexpr Piece& operator=(PieceType _type)
		{
			this->type_ = _type;
			return *this;
		};

	private:

		/**
		 * @brief The type of piece.
		*/
		Type type_;

		/**
		 * @brief The color of the piece.
		*/
		Color color_;
	};



	/**
	 * @brief Represents a piece actually on a chess board.
	*/
	class BoardPiece : public Piece
	{
	public:
		constexpr Position position() const
		{
			return this->pos_;
		};
		constexpr void set_position(Position _pos) noexcept
		{
			this->pos_ = _pos;
		};

		constexpr Rank rank() const noexcept { return this->position().rank(); };
		constexpr File file() const noexcept { return this->position().file(); };

		constexpr void promote(PieceType _type) noexcept
		{
			*this = BoardPiece(_type, this->color(), this->position());
		};

		constexpr BoardPiece() = default;
		constexpr BoardPiece(Type _type, Color _color, Position _pos) :
			Piece(_type, _color), pos_(_pos)
		{};
	private:
		Position pos_;
	};

	/**
	 * @brief Represents a chess board by tracking the pieces directly.
	*/
	class Board
	{
	private:

		auto end() const { return this->pieces_.end(); };

		using size_type = uint64_t;
		constexpr size_type toindex(File _file, Rank _rank) const
		{
			return (jc::to_underlying(_file) << 3) | jc::to_underlying(_rank);
		};
		constexpr size_type toindex(const Position& _pos) const
		{
			return this->toindex(_pos.file(), _pos.rank());
		};
		constexpr Position topos(size_type _index) const
		{
			const auto _rank = Rank(_index & 0b0000'0111);
			const auto _file = File((_index & 0b0011'1000) >> 3);
			return Position(_file, _rank);
		};

		using container_type = std::vector<BoardPiece>;
		using iterator = typename container_type::iterator;
		using const_iterator = typename container_type::const_iterator;

		void erase(const_iterator it)
		{
			this->pieces_by_pos_.at(this->toindex(it->position())) =
				PieceType::none;
			this->pieces_.erase(it);
		};

	public:

		auto find(Position _pos)
		{
			return std::ranges::find_if(this->pieces_, [_pos](BoardPiece& p)
				{
					return p.position() == _pos;
				});
		};
		auto find(Position _pos) const
		{
			return std::ranges::find_if(this->pieces_, [_pos](const BoardPiece& p)
				{
					return p.position() == _pos;
				});
		};

		auto find(Piece _piece)
		{
			return std::ranges::find(this->pieces_, _piece);
		};
		auto find(Piece _piece) const
		{
			return std::ranges::find(this->pieces_, _piece);
		};

		void clear() noexcept
		{
			this->pieces_.clear();
			this->pieces_by_pos_.fill(Piece{});
		};

		void new_piece(Piece _piece, Position _pos)
		{
			this->pieces_.push_back(BoardPiece(_piece.type(), _piece.color(), _pos));
			this->pieces_by_pos_.at(this->toindex(_pos)) = _piece;
		};
		void new_piece(PieceType _piece, Color _color, Position _pos)
		{
			this->new_piece(Piece(_piece, _color), _pos);
		};

		Piece get(Position _pos) const
		{
			return this->pieces_by_pos_.at(this->toindex(_pos));
		};
		auto get(File _file, Rank _rank) const
		{
			return this->get((_file, _rank));
		};

		bool has_enemy_piece(Position _pos, Color _myColor) const
		{
			if (const auto p = this->get(_pos); p)
			{
				return p.color() != _myColor;
			}
			else
			{
				return false;
			};
		};
		bool has_friendy_piece(Position _pos, Color _myColor) const
		{
			if (const auto p = this->get(_pos); p)
			{
				return p.color() == _myColor;
			}
			else
			{
				return false;
			};
		};
		bool has_piece(Position _pos) const
		{
			return (bool)this->get(_pos);
		};
		bool has_enemy_piece_or_empty(Position _pos, Color _myColor) const
		{
			if (const auto p = this->get(_pos); p)
			{
				return p.color() != _myColor;
			}
			else
			{
				return true;
			};
		};

		void erase_piece(Position _position)
		{
			auto it = this->find(_position);
			if (it != this->end())
			{
				this->erase(it);
			};
		};
		void move_piece(Position _fromPos, Position _toPos, PieceType _promotion = PieceType::none)
		{
			this->enpassant_target_.reset();

			{
				const auto _from = this->get(_fromPos);
				const auto _to = this->get(_toPos);

				if (_from == PieceType::king)
				{
					if (_from.color() == Color::white && _fromPos == (File::e, Rank::r1))
					{
						if (_toPos == (File::c, Rank::r1))
						{
							this->find((File::a, Rank::r1))->set_position((File::d, Rank::r1));
						}
						else if (_toPos == (File::g, Rank::r1))
						{
							this->find((File::h, Rank::r1))->set_position((File::f, Rank::r1));
						};
					}
					else if (_from.color() == Color::black && _fromPos == (File::e, Rank::r8))
					{
						if (_toPos == (File::c, Rank::r8))
						{
							this->find((File::a, Rank::r8))->set_position((File::d, Rank::r8));
						}
						else if (_toPos == (File::g, Rank::r8))
						{
							this->find((File::h, Rank::r8))->set_position((File::f, Rank::r8));
						};
					};
				}
				else if (_from.type() == PieceType::pawn)
				{
					if (_from.color() == Color::white)
					{
						if (_fromPos.rank() == Rank::r2 &&
							_toPos.rank() == Rank::r4)
						{
							this->enpassant_target_ = Position(_fromPos.file(), Rank::r3);
						};
					}
					else
					{
						if (_fromPos.rank() == Rank::r7 &&
							_toPos.rank() == Rank::r5)
						{
							this->enpassant_target_ = Position(_fromPos.file(), Rank::r6);
						};
					};
				};

			};

			if (const auto it = this->find(_toPos); it != this->end())
			{
				this->erase(it);
			};

			auto it = this->find(_fromPos);
			assert(it != this->end());

			if (_promotion != PieceType::none && *it == PieceType::pawn)
			{
				it->promote(_promotion);
			};

			it->set_position(_toPos);
			this->pieces_by_pos_.at(this->toindex(_toPos)) = *it;
		};

		void move(const PieceMove& _move)
		{
			this->move_piece(_move.from(), _move.to());
		};
		void move(const Move& _move)
		{
			this->move_piece(_move.from(), _move.to(), _move.promotion());
		};


		bool has_enpassant_target() const noexcept
		{
			return this->enpassant_target_.has_value();
		};
		Position enpassant_target() const noexcept
		{
			return this->enpassant_target_.value();
		};

		const auto& pieces() const { return this->pieces_; }

		friend std::ostream& operator<<(std::ostream& _ostr, const Board& _value);

		Board() = default;

	private:
		container_type pieces_;
		std::array<Piece, 64> pieces_by_pos_{};

		std::optional<Position> enpassant_target_;

	};


	/**
	 * @brief Resets a board to the standard chess starting positions.
	 * @param _board The board to reset.
	 * @return The given board, now reset.
	*/
	inline Board& reset_board(Board& _board)
	{
		// Clear the board so we can begin anew.
		_board.clear();

		// Set black positions
		
		// King and queen
		_board.new_piece(PieceType::king, Color::black, (File::e, Rank::r8));
		_board.new_piece(PieceType::queen, Color::black, (File::d, Rank::r8));

		// Rooks + Minor pieces
		_board.new_piece(PieceType::rook, Color::black, (File::a, Rank::r8));
		_board.new_piece(PieceType::rook, Color::black, (File::h, Rank::r8));
		_board.new_piece(PieceType::knight, Color::black, (File::b, Rank::r8));
		_board.new_piece(PieceType::knight, Color::black, (File::g, Rank::r8));
		_board.new_piece(PieceType::bishop, Color::black, (File::c, Rank::r8));
		_board.new_piece(PieceType::bishop, Color::black, (File::f, Rank::r8));

		// Pawns
		for (auto it = FileIterator(); it != FileIterator::end(); ++it)
		{
			_board.new_piece(PieceType::pawn, Color::black, (*it, Rank::r7));
		};





		// Set white positions

		// King and queen
		_board.new_piece(PieceType::king, Color::white, (File::e, Rank::r1));
		_board.new_piece(PieceType::queen, Color::white, (File::d, Rank::r1));

		// Rooks + Minor pieces
		_board.new_piece(PieceType::rook, Color::white, (File::a, Rank::r1));
		_board.new_piece(PieceType::rook, Color::white, (File::h, Rank::r1));
		_board.new_piece(PieceType::knight, Color::white, (File::b, Rank::r1));
		_board.new_piece(PieceType::knight, Color::white, (File::g, Rank::r1));
		_board.new_piece(PieceType::bishop, Color::white, (File::c, Rank::r1));
		_board.new_piece(PieceType::bishop, Color::white, (File::f, Rank::r1));

		// Pawns
		for (auto it = FileIterator(); it != FileIterator::end(); ++it)
		{
			_board.new_piece(PieceType::pawn, Color::white, (*it, Rank::r2));
		};


		return _board;
	};



	class IGame
	{
	public:

		const Board& board() const
		{
			return this->board_;
		};

		Board board_;
		
		IGame() = default;
	protected:
		~IGame() = default;
	};

	class IChessEngine
	{
	public:

		virtual Move play_turn(IGame& _game) = 0;

		IChessEngine() = default;
		virtual ~IChessEngine() = default;
	};
};
