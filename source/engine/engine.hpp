#pragma once

/** @file */

#include "chess/chess.hpp"

namespace sch
{
	class ScreepFish : public chess::IChessEngine
	{
	public:

		chess::Move play_turn(chess::IGame& _game) final;

		void set_color(chess::Color _color);
		void set_board(chess::Board _board);




		ScreepFish() = default;

	private:
		chess::Board board_;
		chess::Color my_color_;
	};

};