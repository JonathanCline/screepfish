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
	class Move
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



		constexpr Move() = default;
		constexpr Move(Position _from, Position _to) :
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
	constexpr std::string_view fromstr(std::string_view _str, Move& _value)
	{
		// Parse individual positions.
		Position _from, _to;
		_str = fromstr(_str, _from);
		_str = fromstr(_str, _to);

		// Write parsed move.
		_value = Move(_from, _to);
		return _str;
	};

	std::ostream& operator<<(std::ostream& _ostr, const Move& _value);
};
