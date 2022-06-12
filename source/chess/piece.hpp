#pragma once

/** @file */

#include "position.hpp"

#include <jclib/type.h>
#include <jclib/type_traits.h>

#include <array>
#include <iosfwd>
#include <string_view>

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
	 * @brief Array containing the different possible color values.
	*/
	constexpr inline auto colors_v = std::array
	{
		Color::black,
		Color::white
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
		return Color(!static_cast<bool>(rhs));
	};


	constexpr Color square_color(Position _pos)
	{
		const auto a = static_cast<size_t>(_pos);
		const auto b = (((int)_pos.rank() % 2) == 0);
		const auto c = (((int)_pos.file() % 2) == 0);
		return (b ^ c) ? Color::white : Color::black;
	};


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

		/**
		 * @brief Checks if null.
		 * @return True if null, false if NOT null.
		*/
		constexpr bool is_null() const noexcept
		{
			return this->from() == this->to();
		};

		/**
		 * @brief Checks if NOT null.
		 * @return True if NOT null, false if null.
		*/
		constexpr explicit operator bool() const
		{
			return !this->is_null();
		};

		constexpr friend bool operator==(const PieceMove& lhs, jc::null_t)
		{
			return lhs.is_null();
		};
		constexpr friend bool operator==(jc::null_t, const PieceMove& rhs)
		{
			return rhs.is_null();
		};
		constexpr friend bool operator!=(const PieceMove& lhs, jc::null_t)
		{
			return !lhs.is_null();
		};
		constexpr friend bool operator!=(jc::null_t, const PieceMove& rhs)
		{
			return !rhs.is_null();
		};

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

	constexpr inline auto piece_types_v = std::array
	{
		PieceType::none,
		PieceType::pawn,
		PieceType::knight,
		PieceType::bishop,
		PieceType::rook,
		PieceType::queen,
		PieceType::king,
	};

	/**
	 * @brief Holds a move from one position to another along with promotion info.
	*/
	class Move : public PieceMove
	{
	public:



		constexpr PieceType promotion() const
		{
			return this->promotion_;
		};

		constexpr Move() = default;
		constexpr Move(PieceMove _move) :
			PieceMove(_move), promotion_(PieceType::queen)
		{};
		constexpr Move(PieceMove _move, PieceType _promotion) :
			PieceMove(_move), promotion_(_promotion)
		{};
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
			switch (c)
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






	enum class PieceE : uint8_t
	{
		black_pawn = 0b0010,
		black_knight = 0b0100,
		black_bishop = 0b0110,
		black_rook = 0b1000,
		black_queen = 0b1010,
		black_king = 0b1100,

		white_pawn = 0b0011,
		white_knight = 0b0101,
		white_bishop = 0b0111,
		white_rook = 0b1001,
		white_queen = 0b1011,
		white_king = 0b1101,
	};



	/**
	 * @brief Represents a chess piece with defined color.
	*/
	class Piece
	{
	public:

		/**
		 * @brief Exposes the piece type enum values for use
		*/
		using enum PieceType;

		using PieceE = PieceE;
		using enum chess::PieceE;

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
			return Type((this->piece_ & 0b1111'1110) >> 1);
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
			return Color(this->piece_ & 0x1);
		};

		constexpr bool is_white() const
		{
			return this->piece_ & 0x1;
		};

		constexpr explicit operator uint8_t() const noexcept { return this->piece_; };
		constexpr operator PieceE() const noexcept { return PieceE(this->piece_); };


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





		constexpr Piece() noexcept :
			piece_(0)
		{};
		constexpr Piece(Type _type, Color _color) noexcept :
			piece_((jc::to_underlying(_type) << 1) | ((_color == Color::white) ? 0b1 : 0b0))
		{
			if (_type == Type::none) { abort(); };
		};

		constexpr Piece& operator=(PieceType _type)
		{
			if (_type == PieceType::none)
			{
				this->piece_ = 0;
			}
			else
			{
				this->piece_ = (jc::to_underlying(_type) << 1) | (this->piece_ & 0x1);
			};
			return *this;
		};

	private:

		/**
		 * @brief The piece type with color value.
		*/
		uint8_t piece_;

	};

	std::ostream& operator<<(std::ostream& _ostr, const Piece& p);



	/**
	 * @brief Represents a piece with defined color and defined position on a chess board.
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

		friend constexpr inline bool operator==(const BoardPiece& lhs, const Position& rhs) noexcept
		{
			return lhs.position() == rhs;
		};
		friend constexpr inline bool operator==(const Position& lhs, const BoardPiece& rhs) noexcept
		{
			return lhs == rhs.position();
		};
		friend constexpr inline bool operator!=(const BoardPiece& lhs, const Position& rhs) noexcept
		{
			return lhs.position() != rhs;
		};
		friend constexpr inline bool operator!=(const Position& lhs, const BoardPiece& rhs) noexcept
		{
			return lhs != rhs.position();
		};

		constexpr void promote(PieceType _type) noexcept
		{
			*this = BoardPiece(_type, this->color(), this->position());
		};

		constexpr BoardPiece() :
			Piece(),
			pos_{}
		{}
		constexpr BoardPiece(Piece _piece, Position _pos) noexcept :
			Piece(_piece), pos_(_pos)
		{};
		constexpr BoardPiece(Type _type, Color _color, Position _pos) noexcept :
			BoardPiece(Piece(_type, _color), _pos)
		{};

		constexpr BoardPiece& operator=(Piece _piece) noexcept
		{
			Piece::operator=(_piece);
			return *this;
		};
		constexpr BoardPiece& operator=(PieceType _piece) noexcept
		{
			Piece::operator=(_piece);
			return *this;
		};

	private:
		Position pos_{};
	};

};