#pragma once

/** @file */

#include "bitboard.hpp"
#include "board.hpp"


#include <array>
#include <ranges>

namespace chess
{

	consteval BitBoardCX compute_pawn_attack_squares(Position _pos, Color _color)
	{
		auto bb = BitBoardCX();
		if (_pos.file() != File::a)
		{
			if (_color == Color::white)
			{
				bb.set(next(_pos, -1, 1));
			}
			else
			{
				bb.set(next(_pos, -1, -1));
			};
		};
		if (_pos.file() != File::h)
		{
			if (_color == Color::white)
			{
				bb.set(next(_pos, 1, 1));
			}
			else
			{
				bb.set(next(_pos, 1, -1));
			};
		};
		return bb;
	};
	consteval auto compute_pawn_attack_squares(Color _color)
	{
		std::array<BitBoardCX, 64> bbs{};
		auto it = bbs.begin();
		for (auto& v : positions_v)
		{
			if (v.rank() == Rank::r1 || v.rank() == Rank::r8)
			{
				++it;
				continue;
			};
			*it = compute_pawn_attack_squares(v, _color);
			++it;
		};
		return bbs;
	};

	// Precompute attack squares
	constexpr inline auto white_pawn_attack_squares_v = compute_pawn_attack_squares(Color::white);
	constexpr inline auto black_pawn_attack_squares_v = compute_pawn_attack_squares(Color::black);

	constexpr inline auto get_pawn_attacking_squares(Position _pos, Color _color)
	{
		if (_color == Color::white)
		{
			return white_pawn_attack_squares_v[static_cast<size_t>(_pos)];
		}
		else
		{
			return black_pawn_attack_squares_v[static_cast<size_t>(_pos)];
		};
	};






	constexpr BitBoardCX make_rank_bits(Rank _rank)
	{
		auto bb = BitBoardCX();
		for (auto& v : files_v)
		{
			bb.set(v, _rank);
		};
		return bb;
	};
	constexpr BitBoardCX make_file_bits(File _file)
	{
		auto bb = BitBoardCX();
		for (auto& v : ranks_v)
		{
			bb.set(_file, v);
		};
		return bb;
	};
	constexpr BitBoardCX make_file_bits(File _file, Rank _min, Rank _max)
	{
		auto bb = BitBoardCX();
		for (Rank r = _min; r <= _max; r += 1)
		{
			bb.set(_file, r);
		};
		return bb;
	};

	constexpr BitBoardCX make_bits_in_direction(Position _startPos, int df, int dr)
	{
		auto bb = BitBoardCX();

		bool _possible = false;
		auto _nextPos = trynext(_startPos, df, dr, _possible);
		while (_possible)
		{
			bb.set(_nextPos);
			_nextPos = trynext(_nextPos, df, dr, _possible);
		};
		return bb;
	};
	constexpr BitBoardCX make_diagonal_bits(Position _pos)
	{
		auto bb = BitBoardCX();
		const auto _directions = std::array
		{
			std::pair{ 1, 1 },
			std::pair{ 1, -1 },
			std::pair{ -1, 1 },
			std::pair{ -1, -1 }
		};
		for (auto& v : _directions)
		{
			bb |= make_bits_in_direction(_pos, v.first, v.second);
		};
		return bb;
	};



	/**
	 * @brief Fixed max sized array that acts like a vector for positions.
	*/
	template <size_t Max>
	struct FixedPositionVector
	{
	public:

		constexpr void append(Position p)
		{
			this->data_[this->count_++] = p;
		};
		constexpr void append(File f, Rank r)
		{
			this->append(Position(f, r));
		};

		template <std::ranges::range RangeT>
		constexpr void append(RangeT&& _range)
		{
			for (auto& v : _range)
			{
				this->append(v);
			};
		};

		constexpr auto begin()
		{
			return this->data_.begin();
		};
		constexpr auto begin() const
		{
			return this->data_.begin();
		};

		constexpr auto end()
		{
			return std::next(this->begin(), this->count_);
		};
		constexpr auto end() const
		{
			return std::next(this->begin(), this->count_);
		};

		constexpr FixedPositionVector() :
			data_{},
			count_(0)
		{};

	private:
		std::array<Position, Max> data_;
		uint8_t count_;
	};


	namespace impl
	{
		consteval FixedPositionVector<7>
			compute_threat_positions_in_direction(Position _pos, int df, int dr)
		{
			auto _arr = FixedPositionVector<7>{};
			bool _possible = false;
			auto _nextPos = trynext(_pos, df, dr, _possible);
			while (_possible)
			{
				_arr.append(_nextPos);
				_nextPos = trynext(_nextPos, df, dr, _possible);
			};
			return _arr;
		};
	};

	/**
	 * @brief Computes the positions that an enemy piece MAY attack a position from.
	 * @param _pos Position that could be attacked.
	 * @return Positions that could be attacking it.
	*/
	consteval FixedPositionVector<27> compute_threat_positions(Position _pos)
	{
		auto _arr = FixedPositionVector<27>{};
		
		// Positions in rank
		for (auto& r : ranks_v)
		{
			if (r != _pos.rank())
			{
				_arr.append(_pos.file(), r);
			};
		};

		// Positions in file
		for (auto& f : files_v)
		{
			if (f != _pos.file())
			{
				_arr.append(f, _pos.rank());
			};
		};

		// Position on diagonals
		_arr.append(impl::compute_threat_positions_in_direction(_pos, +1, +1));
		_arr.append(impl::compute_threat_positions_in_direction(_pos, +1, -1));
		_arr.append(impl::compute_threat_positions_in_direction(_pos, -1, +1));
		_arr.append(impl::compute_threat_positions_in_direction(_pos, -1, -1));

		return _arr;
	};

	/**
	 * @brief Computes the positions that an enemy piece MAY attack a position for all positions.
	 * @return Threat positions for each board position.
	*/
	consteval auto compute_threat_positions()
	{
		auto _arr = std::array<FixedPositionVector<27>, 64>{};
		for (auto& _pos : positions_v)
		{
			_arr[static_cast<size_t>(_pos)] = compute_threat_positions(_pos);
		};
		return _arr;
	};
	
	/**
	 * @brief Lookup table with the positions that an enemy piece MAY attack a position for all positions.
	*/
	constexpr auto threat_positions_v = compute_threat_positions();

};
