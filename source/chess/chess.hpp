#pragma once

/** @file */

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

namespace chess
{
	/**
	 * @brief Named chess board files.
	*/
	enum class File : uint8_t
	{
		a = 0,
		b = 1,
		c = 2,
		d = 3,
		e = 4,
		f = 5,
		g = 6,
		h = 7
	};

	/**
	 * @brief Gets the character representation for a file.
	 * @param _file Chess board file.
	 * @return Character rep for the given file.
	*/
	constexpr char tochar(File _file) noexcept
	{
		switch (_file)
		{
		case File::a:
			return 'a';
		case File::b:
			return 'b';
		case File::c:
			return 'c';
		case File::d:
			return 'd';
		case File::e:
			return 'e';
		case File::f:
			return 'f';
		case File::g:
			return 'g';
		case File::h:
			return 'h';
		default:
			SCREEPFISH_UNREACHABLE;
		};
	};

	/**
	 * @brief Converts a character representation into a file value.
	 * @param c Character to convert.
	 * @param _file File out value.
	*/
	constexpr void fromchar(char c, File& _file) noexcept
	{
		_file = File(c - 'a');
	};

	// Writes the given file to the output stream.
	std::ostream& operator<<(std::ostream& _ostr, const File& _value);
	std::istream& operator>>(std::istream& _ostr, File& _value);


	/**
	 * @brief Iterates through chess board files.
	*/
	class FileIterator
	{
	public:
		using value_type = File;

		constexpr static FileIterator end() noexcept
		{
			return FileIterator(jc::to_underlying(File::h) + 1);
		};
		constexpr auto operator<=>(const FileIterator& rhs) const = default;

		constexpr value_type operator*() const
		{
			const auto o = value_type(this->file_);
			assert(o < this->end());
			return o;
		};
		constexpr FileIterator& operator++()
		{
			assert(value_type(this->file_) <= File::h);
			++this->file_;
			return *this;
		};

		constexpr FileIterator() = default;
		constexpr FileIterator(value_type _file) :
			FileIterator(jc::to_underlying(_file))
		{};

	private:
		constexpr explicit FileIterator(std::underlying_type_t<File> _fileVal) noexcept :
			file_(_fileVal)
		{
			assert(_fileVal <= jc::to_underlying(File::h) + 1);
		};
		
		uint8_t file_;
	};





	/**
	 * @brief Named chess board ranks.
	*/
	enum class Rank : uint8_t
	{
		r1 = 0,
		r2 = 1,
		r3 = 2,
		r4 = 3,
		r5 = 4,
		r6 = 5,
		r7 = 6,
		r8 = 7
	};

	/**
	 * @brief Gets the character representation for a rank.
	 * @param _rank Chess board rank.
	 * @return Character rep for the given rank.
	*/
	constexpr char tochar(Rank _rank) noexcept
	{
		switch (_rank)
		{
		case Rank::r1:
			return '1';
		case Rank::r2:
			return '2';
		case Rank::r3:
			return '3';
		case Rank::r4:
			return '4';
		case Rank::r5:
			return '5';
		case Rank::r6:
			return '6';
		case Rank::r7:
			return '7';
		case Rank::r8:
			return '8';
		default:
			SCREEPFISH_UNREACHABLE;
		};
	};

	/**
	 * @brief Converts a character representation into a rank value.
	 * @param c Character to convert.
	 * @param _rank Rank out value.
	*/
	constexpr void fromchar(char c, Rank& _rank) noexcept
	{
		_rank = Rank(c - '1');
	};

	std::ostream& operator<<(std::ostream& _ostr, const Rank& _value);
	std::istream& operator>>(std::istream& _ostr, Rank& _value);




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







	/**
	 * @brief Holds a position on a chess board as a file/rank pair
	*/
	class Position
	{
	public:

		/**
		 * @brief Gets the rank of this position.
		*/
		constexpr Rank rank() const noexcept { return this->rank_; };
		
		/**
		 * @brief Gets the file of this position.
		*/
		constexpr File file() const noexcept { return this->file_; };

		constexpr bool operator==(const Position& rhs) const noexcept = default;
		constexpr bool operator!=(const Position& rhs) const noexcept = default;

		constexpr Position() noexcept = default;
		constexpr Position(File _file, Rank _rank) noexcept :
			file_(_file),
			rank_(_rank)
		{};

	private:
		File file_;
		Rank rank_;
	};

	/**
	 * @brief Allows more natural construction of a position value from a file, rank pair.
	 * 
	 * Example of usage:
	 *		Position p = (File::a, Rank::r4);
	 * 
	 * @param _file File of the position.
	 * @param _rank Rank of the position.
	 * @return Combined file, rank pair as a position.
	*/
	constexpr Position operator,(File _file, Rank _rank) noexcept
	{
		return Position(_file, _rank);
	};

	/**
	 * @brief Parses a position value from a string.
	 * 
	 * @return String with parsed characters removed.
	*/
	constexpr std::string_view fromstr(std::string_view _str, Position& _value)
	{
		if (_str.size() < 2)
		{
			// fail
			abort();
		};

		const auto _fileChar = _str.front();
		if (!inbounds(_fileChar, 'a', 'h', inclusive))
		{
			// fail
			abort();
		};

		const auto _rankChar = _str[1];
		if (!inbounds(_rankChar, '1', '8', inclusive))
		{
			// fail
			abort();
		};

		// Convert chars to rank and file
		auto _file = File();
		auto _rank = Rank();
		fromchar(_fileChar, _file);
		fromchar(_rankChar, _rank);

		// Write parsed position
		_value = Position(_file, _rank);

		// Return string with parsed section remove
		_str.remove_prefix(2);
		return _str;
	};
	

	std::ostream& operator<<(std::ostream& _ostr, const Position& _value);


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
	 * @brief Represents a chess board by tracking the pieces directly.
	*/
	class Board
	{
	private:

		class PieceInfo : public Piece
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
				*this = PieceInfo(_type, this->color(), this->position());
			};

			constexpr PieceInfo(Type _type, Color _color, Position _pos) :
				Piece(_type, _color), pos_(_pos)
			{};
		private:
			Position pos_;
		};

		auto find(Piece _piece)
		{
			return std::ranges::find(this->pieces_, _piece);
		};
		auto find(Piece _piece) const
		{
			return std::ranges::find(this->pieces_, _piece);
		};

		auto end() const { return this->pieces_.end(); };
		
		auto find(Position _pos)
		{
			return std::ranges::find_if(this->pieces_, [_pos](PieceInfo& p)
				{
					return p.position() == _pos;
				});
		};
		auto find(Position _pos) const
		{
			return std::ranges::find_if(this->pieces_, [_pos](const PieceInfo& p)
				{
					return p.position() == _pos;
				});
		};

		Piece at(Position _pos) const
		{
			if (const auto it = this->find(_pos); it != this->end())
			{
				return *it;
			}
			else
			{
				return Piece{};
			};
		};

	public:
		
		void clear() noexcept { this->pieces_.clear(); };

		void new_piece(Piece _piece, Position _pos)
		{
			this->pieces_.push_back(PieceInfo(_piece.type(), _piece.color(), _pos));
		};
		void new_piece(PieceType _piece, Color _color, Position _pos)
		{
			this->pieces_.push_back(PieceInfo(_piece, _color, _pos));
		};

		void erase_piece(Position _position)
		{
			auto it = this->find(_position);
			if (it != this->end())
			{
				this->pieces_.erase(it);
			};
		};
		void move_piece(Position _fromPos, Position _toPos, PieceType _promotion = PieceType::none)
		{
			{
				const auto _from = this->at(_fromPos);
				const auto _to = this->at(_toPos);

				if (_from && _from == PieceType::king)
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
				};
			};

			if (const auto it = this->find(_toPos); it != this->end())
			{
				this->pieces_.erase(it);
			};

			auto it = this->find(_fromPos);
			assert(it != this->end());

			if (_promotion != PieceType::none && *it == PieceType::pawn)
			{
				it->promote(_promotion);
			};

			it->set_position(_toPos);
		};
		
		friend std::ostream& operator<<(std::ostream& _ostr, const Board& _value);

		Board() = default;

	private:
		std::vector<PieceInfo> pieces_;
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

};
