#pragma once

/** @file */

#include "position.hpp"

#include <bitset>
#include <iostream>

namespace chess
{
	class BitBoardCX;
	
	class BitBoard
	{
	private:
		friend BitBoardCX;
		
		using size_type = uint64_t;

		
		constexpr size_type toindex(Position _pos) const
		{
			return static_cast<size_type>(_pos);
		};
		constexpr size_type toindex(File _file, Rank _rank) const
		{
			return this->toindex((_file, _rank));
		};

		constexpr Position topos(size_type _index) const
		{
			const auto _rank = Rank(_index & 0b0000'0111);
			const auto _file = File((_index & 0b0011'1000) >> 3);
			return Position(_file, _rank);
		};

	public:

		void set(Position _pos)
		{
			this->bits_.set(this->toindex(_pos));
		};
		void set(File _file, Rank _rank)
		{
			this->bits_.set(this->toindex(_file, _rank));
		};
		
		void reset(Position _pos)
		{
			this->bits_.reset(this->toindex(_pos));
		};
		void reset(File _file, Rank _rank)
		{
			this->bits_.reset(this->toindex(_file, _rank));
		};
		void reset()
		{
			this->bits_.reset();
		};

		void set(Position _pos, bool _value)
		{
			if (_value)
			{
				this->set(_pos);
			}
			else
			{
				this->reset(_pos);
			};
		};
		void set(File _file, Rank _rank, bool _value)
		{
			if (_value)
			{
				this->set(_file, _rank);
			}
			else
			{
				this->reset(_file, _rank);
			};
		};

		bool test(Position _pos) const
		{
			return this->bits_.test(this->toindex(_pos));
		};
		bool test(File _file, Rank _rank) const
		{
			return this->bits_.test(this->toindex(_file, _rank));
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

		BitBoard operator~() const
		{
			auto& lhs = *this;
			return BitBoard(~lhs.bits_);
		};

		BitBoard operator|(const BitBoard& rhs) const
		{
			auto& lhs = *this;
			return BitBoard(lhs.bits_ | rhs.bits_);
		};
		BitBoard operator&(const BitBoard& rhs) const
		{
			auto& lhs = *this;
			return BitBoard(lhs.bits_ & rhs.bits_);
		};
		BitBoard operator^(const BitBoard& rhs) const
		{
			auto& lhs = *this;
			return BitBoard(lhs.bits_ ^ rhs.bits_);
		};

		BitBoard& operator|=(const BitBoard& rhs)
		{
			auto& lhs = *this;
			lhs.bits_ |= rhs.bits_;
			return lhs;
		};
		BitBoard& operator&=(const BitBoard& rhs)
		{
			auto& lhs = *this;
			lhs.bits_ &= rhs.bits_;
			return lhs;
		};
		BitBoard& operator^=(const BitBoard& rhs)
		{
			auto& lhs = *this;
			lhs.bits_ ^= rhs.bits_;
			return lhs;
		};

		bool operator==(const BitBoard& rhs) const
		{
			auto& lhs = *this;
			return lhs.bits_ == rhs.bits_;
		};
		bool operator!=(const BitBoard& rhs) const
		{
			auto& lhs = *this;
			return lhs.bits_ != rhs.bits_;
		};

		BitBoard() = default;
	private:
		explicit BitBoard(std::bitset<64> _bits) :
			bits_(_bits)
		{};

		std::bitset<64> bits_;
	};

	std::ostream& operator<<(std::ostream& _ostr, const BitBoard& _value);



	class BitBoardCX
	{
	private:
		using size_type = uint64_t;
		
		constexpr size_type toindex(Position _pos) const
		{
			return static_cast<size_type>(_pos);
		};
		constexpr size_type toindex(File _file, Rank _rank) const
		{
			return this->toindex(Position(_file, _rank));
		};
		

		constexpr Position topos(size_type _index) const
		{
			const auto _rank = Rank(_index  & 0b0000'0111);
			const auto _file = File((_index & 0b0011'1000) >> 3);
			return Position(_file, _rank);
		};

		constexpr size_type bitmask(size_type _index) const
		{
			return size_type(1) << _index;
		};
		constexpr size_type bitmask(Position _pos) const
		{
			return this->bitmask(this->toindex(_pos));
		};
		constexpr size_type bitmask(File _file, Rank _rank) const
		{
			return this->bitmask(this->toindex(_file, _rank));
		};

	public:

		operator BitBoard() const noexcept
		{
			return BitBoard(this->bits_);
		};

		constexpr void set(File _file, Rank _rank)
		{
			this->bits_ |= this->bitmask(_file, _rank);
		};
		constexpr void set(Position _pos)
		{
			this->bits_ |= this->bitmask(_pos);
		};

		constexpr void reset(File _file, Rank _rank)
		{
			this->bits_ &= ~this->bitmask(_file, _rank);
		};
		constexpr void reset()
		{
			this->bits_ = 0;
		};

		constexpr void set(File _file, Rank _rank, bool _value)
		{
			if (_value)
			{
				this->set(_file, _rank);
			}
			else
			{
				this->reset(_file, _rank);
			};
		};
	
		constexpr bool test(File _file, Rank _rank) const
		{
			return this->bits_ & this->bitmask(_file, _rank);
		};
		constexpr bool test(Position _pos) const
		{
			return this->bits_ & this->bitmask(_pos);
		};

		constexpr bool all() const
		{
			return this->bits_ == 0xFFFF'FFFF'FFFF'FFFF;
		};
		constexpr bool any() const
		{
			return this->bits_ != 0;
		};
		constexpr bool none() const
		{
			return this->bits_ == 0;
		};

		constexpr BitBoardCX operator~() const
		{
			auto& lhs = *this;
			return BitBoardCX(~lhs.bits_);
		};

		constexpr BitBoardCX operator|(const BitBoardCX& rhs) const
		{
			auto& lhs = *this;
			return BitBoardCX(lhs.bits_ | rhs.bits_);
		};
		constexpr BitBoardCX operator&(const BitBoardCX& rhs) const
		{
			auto& lhs = *this;
			return BitBoardCX(lhs.bits_ & rhs.bits_);
		};
		constexpr BitBoardCX operator^(const BitBoardCX& rhs) const
		{
			auto& lhs = *this;
			return BitBoardCX(lhs.bits_ ^ rhs.bits_);
		};

		constexpr BitBoardCX& operator|=(const BitBoardCX& rhs)
		{
			auto& lhs = *this;
			lhs.bits_ |= rhs.bits_;
			return lhs;
		};
		constexpr BitBoardCX& operator&=(const BitBoardCX& rhs)
		{
			auto& lhs = *this;
			lhs.bits_ &= rhs.bits_;
			return lhs;
		};
		constexpr BitBoardCX& operator^=(const BitBoardCX& rhs)
		{
			auto& lhs = *this;
			lhs.bits_ ^= rhs.bits_;
			return lhs;
		};

		constexpr bool operator==(const BitBoardCX& rhs) const
		{
			auto& lhs = *this;
			return lhs.bits_ == rhs.bits_;
		};
		constexpr bool operator!=(const BitBoardCX& rhs) const
		{
			auto& lhs = *this;
			return lhs.bits_ != rhs.bits_;
		};

		constexpr BitBoardCX() = default;
	private:
		constexpr explicit BitBoardCX(uint64_t _bits) :
			bits_(_bits)
		{};
		
		uint64_t bits_;
	};
}

