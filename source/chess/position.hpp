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
#include <optional>

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
		if (_newFile > File::h) JCLIB_UNLIKELY
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
		if (_newRank > Rank::r8) JCLIB_UNLIKELY
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
	 * @brief Holds an offset as a delta rank, delta file pair.
	*/
	class Offset
	{
	public:
		using rep = int8_t;

		constexpr rep delta_rank() const noexcept { return this->dr_; };
		constexpr rep delta_file() const noexcept { return this->df_; };

		constexpr Offset operator-() const noexcept
		{
			return Offset(-this->df_, -this->dr_);
		};

		friend constexpr inline Offset operator+(const Offset& lhs, const Offset& rhs) noexcept
		{
			return Offset
			(
				lhs.delta_file() + rhs.delta_file(),
				lhs.delta_rank() + rhs.delta_rank()
			);
		};
		friend constexpr inline Offset& operator+=(Offset& lhs, const Offset& rhs) noexcept
		{
			lhs.df_ += rhs.df_;
			lhs.dr_ += rhs.dr_;
			return lhs;
		};
		friend constexpr inline Offset operator-(const Offset& lhs, const Offset& rhs) noexcept
		{
			return Offset
			(
				lhs.delta_file() - rhs.delta_file(),
				lhs.delta_rank() - rhs.delta_rank()
			);
		};
		friend constexpr inline Offset& operator-=(Offset& lhs, const Offset& rhs) noexcept
		{
			lhs.df_ -= rhs.df_;
			lhs.dr_ -= rhs.dr_;
			return lhs;
		};

		constexpr Offset() = default;
		constexpr Offset(rep _deltaFile, rep _deltaRank) noexcept :
			df_(_deltaFile), dr_(_deltaRank)
		{};

	private:
		rep df_ : 4;
		rep dr_ : 4;
	};




	enum class DirectionBit : uint8_t
	{
		n = 0,

		u = 0b0001,
		l = 0b0010,
		d = 0b0100,
		r = 0b1000,

		ul = 0b0011,
		dl = 0b0110,
		dr = 0b1100,
		ur = 0b1001,
	};
	constexpr DirectionBit operator|(DirectionBit lhs, DirectionBit rhs)
	{
		return DirectionBit(jc::to_underlying(lhs) | jc::to_underlying(rhs));
	};
	constexpr DirectionBit operator&(DirectionBit lhs, DirectionBit rhs)
	{
		return DirectionBit(jc::to_underlying(lhs) & jc::to_underlying(rhs));
	};




	/**
	 * @brief Holds a direction as a pos/neg rank/file pair.
	*/
	class Direction
	{
	public:

		using Dir = DirectionBit;
		using enum DirectionBit;

	private:

		constexpr static DirectionBit make_dir(int8_t df, int8_t dr)
		{
			auto fDir = Dir{};
			auto rDir = Dir{};

			if (df < 0)
			{
				fDir = Dir::l;
			}
			else if (df > 0)
			{
				fDir = Dir::r;
			};

			if (dr < 0)
			{
				rDir = Dir::d;
			}
			else if (dr > 0)
			{
				rDir = Dir::u;
			};

			return rDir | fDir;
		};
		constexpr static DirectionBit opposite(DirectionBit _dir)
		{
			switch (_dir)
			{
			case Dir::n: return Dir::n;
			case Dir::d: return Dir::u;
			case Dir::u: return Dir::d;
			case Dir::l: return Dir::r;
			case Dir::r: return Dir::l;
			case Dir::dl: return Dir::ur;
			case Dir::dr: return Dir::ul;
			case Dir::ul: return Dir::dr;
			case Dir::ur: return Dir::dl;
			};
		};

	public:
		using rep = uint8_t;

		constexpr int8_t delta_rank() const
		{
			switch (this->dir_)
			{
			case Dir::u: [[fallthrough]];
			case Dir::ul: [[fallthrough]];
			case Dir::ur:
				return 1;
			case Dir::d: [[fallthrough]];
			case Dir::dl: [[fallthrough]];
			case Dir::dr:
				return -1;
			default:
				return 0;
			};
		};
		constexpr int8_t delta_file() const
		{
			switch (this->dir_)
			{
			case Dir::l: [[fallthrough]];
			case Dir::ul: [[fallthrough]];
			case Dir::dl:
				return -1;
			case Dir::r: [[fallthrough]];
			case Dir::ur: [[fallthrough]];
			case Dir::dr:
				return 1;
			default:
				return 0;
			};
		};

		constexpr bool pos_rank() const noexcept
		{
			return (this->dir_ & Dir::u) != Dir::n;
		};
		constexpr bool pos_file() const noexcept
		{
			return (this->dir_ & Dir::r) != Dir::n;
		};
		constexpr bool neg_rank() const noexcept
		{
			return (this->dir_ & Dir::d) != Dir::n;
		};
		constexpr bool neg_file() const noexcept
		{
			return (this->dir_ & Dir::l) != Dir::n;
		};

		constexpr Offset offset(int8_t _count) const noexcept
		{
			return Offset(this->delta_file() * _count, this->delta_rank() * _count);
		};
		constexpr Offset offset() const noexcept
		{
			return Offset(this->delta_file(), this->delta_rank());
		};

		constexpr Direction operator-() const noexcept
		{
			return Direction(this->opposite(this->dir_));
		};

		constexpr operator Dir() const noexcept
		{
			return this->dir_;
		};

		constexpr operator Offset() const noexcept
		{
			return this->offset();
		};

		constexpr Direction() = default;
		constexpr Direction(Dir _dir) noexcept :
			dir_(_dir)
		{};
		constexpr Direction(int8_t _fileOff, int8_t _rankOff) noexcept :
			Direction(make_dir(_fileOff, _rankOff))
		{};
		constexpr Direction(const Offset& _offset) noexcept :
			Direction(make_dir(_offset.delta_file(), _offset.delta_rank()))
		{};

	private:

		Dir dir_;
	};



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

		friend constexpr inline bool operator==(const Position& lhs, File rhs) noexcept
		{
			return lhs.file() == rhs;
		};
		friend constexpr inline bool operator==(const Position& lhs, Rank rhs) noexcept
		{
			return lhs.rank() == rhs;
		};
		friend constexpr inline bool operator==(File lhs, const Position& rhs) noexcept
		{
			return lhs == rhs.file();
		};
		friend constexpr inline bool operator==(Rank lhs, const Position& rhs) noexcept
		{
			return lhs == rhs.rank();
		};

		friend constexpr inline bool operator!=(const Position& lhs, File rhs) noexcept
		{
			return lhs.file() != rhs;
		};
		friend constexpr inline bool operator!=(const Position& lhs, Rank rhs) noexcept
		{
			return lhs.rank() != rhs;
		};
		friend constexpr inline bool operator!=(File lhs, const Position& rhs) noexcept
		{
			return lhs != rhs.file();
		};
		friend constexpr inline bool operator!=(Rank lhs, const Position& rhs) noexcept
		{
			return lhs != rhs.rank();
		};

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

	constexpr Offset operator-(const Position& lhs, const Position& rhs) noexcept
	{
		return Offset
		(
			jc::to_underlying(lhs.file()) - jc::to_underlying(rhs.file()),
			jc::to_underlying(lhs.rank()) - jc::to_underlying(rhs.rank())
		);
	};
	
	constexpr Position operator+(const Position& lhs, const Offset& rhs) noexcept
	{
		return Position
		(
			lhs.file() + rhs.delta_file(),
			lhs.rank() + rhs.delta_rank()
		);
	};
	constexpr Position operator+(const Offset& lhs, const Position& rhs) noexcept
	{
		return Position
		(
			rhs.file() + lhs.delta_file(),
			rhs.rank() + lhs.delta_rank()
		);
	};

	constexpr Position operator-(const Position& lhs, const Offset& rhs) noexcept
	{
		return Position
		(
			lhs.file() - rhs.delta_file(),
			lhs.rank() - rhs.delta_rank()
		);
	};
	constexpr Position operator-(const Offset& lhs, const Position& rhs) noexcept
	{
		return Position
		(
			rhs.file() - lhs.delta_file(),
			rhs.rank() - lhs.delta_rank()
		);
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

	constexpr inline auto rev_positions_v = ([]()
		{
			auto _pos = std::array<Position, 64>{};
			auto it = _pos.begin();
			for (uint8_t b = 0; b != 64; ++b)
			{
				*it = Position::from_bits(63 - b);
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
		if (!_possible) JCLIB_UNLIKELY { return _position; };

		const auto _newRank = trynext(_position.rank(), _dRank, _possible);
		if (!_possible) JCLIB_UNLIKELY { return _position; };

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
		SCREEPFISH_ASSERT(_str.size() >= 2);

		const auto _fileChar = _str.front();
		SCREEPFISH_ASSERT(inbounds(_fileChar, 'a', 'h', inclusive));

		const auto _rankChar = _str[1];
		SCREEPFISH_ASSERT(inbounds(_rankChar, '1', '8', inclusive));

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




	/**
	 * @brief Parses a position value from a string.
	 *
	 * @return String with parsed characters removed.
	*/
	constexpr std::optional<Position> try_parse_position(std::string_view _str)
	{
		if (_str.size() != 2) { return std::nullopt; };

		const auto _fileChar = _str.front();
		if (!inbounds(_fileChar, 'a', 'h', inclusive)) { return std::nullopt; };
		const auto _rankChar = _str[1];
		if (!inbounds(_rankChar, '1', '8', inclusive)) { return std::nullopt; };

		// Convert chars to rank and file
		auto _file = File();
		auto _rank = Rank();
		fromchar(_fileChar, _file);
		fromchar(_rankChar, _rank);

		// Return parsed position
		return Position(_file, _rank);
	};



	std::ostream& operator<<(std::ostream& _ostr, const Position& _value);





	constexpr int8_t distance(File lhs, File rhs)
	{
		return jc::to_underlying(std::max(lhs, rhs)) - jc::to_underlying(std::min(lhs, rhs));
	};
	constexpr int8_t distance(Rank lhs, Rank rhs)
	{
		return jc::to_underlying(std::max(lhs, rhs)) - jc::to_underlying(std::min(lhs, rhs));
	};

};