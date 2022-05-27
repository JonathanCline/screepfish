#pragma once

/** @file */

#include "utility/string.hpp"
#include "utility/number.hpp"
#include "utility/utility.hpp"

#include <jclib/type_traits.h>
#include <jclib/concepts.h>

#include <cassert>
#include <iosfwd>
#include <cstdint>
#include <string_view>
#include <array>

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

	constexpr auto files_v = std::array
	{
		chess::File::a,
		chess::File::b,
		chess::File::c,
		chess::File::d,
		chess::File::e,
		chess::File::f,
		chess::File::g,
		chess::File::h,
	};

	// File math operators
	constexpr File operator+(File _file, int8_t _count)
	{
		return File(jc::to_underlying(_file) + _count);
	};
	constexpr File operator+(int8_t _count, File _file)
	{
		return File(jc::to_underlying(_file) + _count);
	};
	constexpr File operator-(File _file, int8_t _count)
	{
		return File(jc::to_underlying(_file) - _count);
	};
	constexpr File& operator+=(File& _file, int8_t _count)
	{
		_file = _file + _count;
		return _file;
	};
	constexpr File& operator-=(File& _file, int8_t _count)
	{
		_file = _file - _count;
		return _file;
	};


	/**
	 * @brief Increments the given file if possible.
	 * @param _file File to increment.
	 * @param _count How much to increment.
	 * @return True if possible, false on not possible.
	*/
	constexpr bool trynext(File& _file, int _count = 1)
	{
		auto _newFile = File(jc::to_underlying(_file) + _count);
		if (_newFile > File::h)
		{
			return false;
		}
		else
		{
			_file = _newFile;
			return true;
		};
	};

	/**
	 * @brief Increments the given file if possible.
	 * @param _file File to increment.
	 * @param _count How much to increment.
	 * @param _possible Try/Fail out variable.
	 * @return Incremented file value if possible, or no change.
	*/
	constexpr File trynext(File _file, int _count, bool& _possible)
	{
		_possible = trynext(_file, _count);
		return _file;
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

	constexpr auto ranks_v = std::array
	{
		chess::Rank::r1,
		chess::Rank::r2,
		chess::Rank::r3,
		chess::Rank::r4,
		chess::Rank::r5,
		chess::Rank::r6,
		chess::Rank::r7,
		chess::Rank::r8,
	};

	constexpr auto rev_ranks_v = std::array
	{
		chess::Rank::r8,
		chess::Rank::r7,
		chess::Rank::r6,
		chess::Rank::r5,
		chess::Rank::r4,
		chess::Rank::r3,
		chess::Rank::r2,
		chess::Rank::r1,
	};

	// Rank math operators
	constexpr Rank operator+(Rank _rank, int8_t _count)
	{
		return Rank(jc::to_underlying(_rank) + _count);
	};
	constexpr Rank operator+(int8_t _count, Rank _rank)
	{
		return Rank(jc::to_underlying(_rank) + _count);
	};
	constexpr Rank operator-(Rank _rank, int8_t _count)
	{
		return Rank(jc::to_underlying(_rank) - _count);
	};
	constexpr Rank& operator+=(Rank& _rank, int8_t _count)
	{
		_rank = _rank + _count;
		return _rank;
	};
	constexpr Rank& operator-=(Rank& _rank, int8_t _count)
	{
		_rank = _rank - _count;
		return _rank;
	};


	/**
	 * @brief Increments the given rank if possible.
	 * @param _rank Rank to increment.
	 * @param _count How much to increment.
	 * @return True if possible, false on not possible.
	*/
	constexpr bool trynext(Rank& _rank, int _count = 1)
	{
		auto _newRank = Rank(jc::to_underlying(_rank) + _count);
		if (_newRank > Rank::r8)
		{
			return false;
		}
		else
		{
			_rank = _newRank;
			return true;
		};
	};

	/**
	 * @brief Increments the given rank if possible.
	 * @param _rank Rank to increment.
	 * @param _count How much to increment.
	 * @param _possible Try/Fail out variable.
	 * @return Incremented rank value if possible, or no change.
	*/
	constexpr Rank trynext(Rank _rank, int _count, bool& _possible)
	{
		_possible = trynext(_rank, _count);
		return _rank;
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
	 * @brief Holds a position on a chess board as a file/rank pair
	*/
	class Position
	{
	private:

		static constexpr uint8_t concat(File f, Rank r) noexcept
		{
			return (jc::to_underlying(f) << 3) | jc::to_underlying(r);
		};


	public:

		template <jc::cx_integer T>
		constexpr explicit operator T() const noexcept { return static_cast<T>(this->pos_); };
		
		constexpr static Position from_bits(uint8_t _index) noexcept { return Position(_index); };


		/**
		 * @brief Gets the rank of this position.
		*/
		constexpr Rank rank() const noexcept
		{
			return Rank(this->pos_ & 0b111);
		};

		/**
		 * @brief Gets the file of this position.
		*/
		constexpr File file() const noexcept
		{
			return File((this->pos_ & 0b111000) >> 3);
		};

		constexpr bool operator==(const Position& rhs) const noexcept = default;
		constexpr bool operator!=(const Position& rhs) const noexcept = default;

		constexpr Position() noexcept = default;
		constexpr Position(File _file, Rank _rank) noexcept :
			pos_(concat(_file, _rank))
		{};

		constexpr Position& operator=(File _file) noexcept
		{
			this->pos_ = this->concat(_file, this->rank());
			return *this;
		};
		constexpr Position& operator=(Rank _rank) noexcept
		{
			this->pos_ = this->concat(this->file(), _rank);
			return *this;
		};

	private:
		constexpr explicit Position(uint8_t _index) noexcept :
			pos_(_index)
		{};

		uint8_t pos_;
	};

	constexpr inline auto positions_v = ([]()
		{
			auto _pos = std::array<Position, 64>{};
			auto it = _pos.begin();
			for (uint8_t b = 0; b != 64; ++b)
			{
				*it = Position::from_bits(b);
				++it;
			};
			return _pos;
		})();



	/**
	 * @brief Increments the given position, does not check for validity!
	 * @param _position Position to increment.
	 * @param _dFile How much to increment the file.
	 * @param _dRank How much to increment the rank.
	 * @return Incremented position value.
	*/
	constexpr Position next(Position _position, int _dFile, int _dRank)
	{
		return Position(_position.file() + _dFile, _position.rank() + _dRank);
	};

	/**
	 * @brief Increments the given position if possible.
	 * @param _position Position to increment.
	 * @param _dFile How much to increment the file.
	 * @param _dRank How much to increment the rank.
	 * @param _possible Try/Fail out variable.
	 * @return Incremented position value if possible, or no change.
	*/
	constexpr Position trynext(Position _position, int _dFile, int _dRank, bool& _possible)
	{
		const auto _newFile = trynext(_position.file(), _dFile, _possible);
		if (!_possible) { return _position; };

		const auto _newRank = trynext(_position.rank(), _dRank, _possible);
		if (!_possible) { return _position; };

		return Position(_newFile, _newRank);
	};

	/**
	 * @brief Increments the given position if possible.
	 * @param _position Position to increment.
	 * @param _dFile How much to increment the file.
	 * @param _dRank How much to increment the rank.
	 * @return True if possible, false otherwise.
	*/
	constexpr bool trynext(Position& _position, int _dFile, int _dRank)
	{
		auto _possible = true;
		_position = trynext(_position, _dFile, _dRank, _possible);
		return _possible;
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



};