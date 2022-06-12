#pragma once

/** @file */

#include "bitboard.hpp"
#include "board.hpp"

#include <array>

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

}