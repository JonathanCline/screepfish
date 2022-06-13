#pragma once

/** @file */

#include "chess/chess.hpp"

namespace chess
{
	class BoardViewTerminal
	{
	public:

		void step();
		void wait_for_any_key();

		bool should_close() const;
		void set_board(const chess::Board& _board);





		BoardViewTerminal(const char* _assetsDirectory, bool _step = false);
		~BoardViewTerminal();

	private:
		BoardViewTerminal(const BoardViewTerminal&) = delete;
		BoardViewTerminal(BoardViewTerminal&&) = delete;


		int cw_;
		int ch_;
		bool should_close_ = false;
		bool step_ = false;
	};
};
