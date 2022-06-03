#pragma once

/** @file */

#include "chess/chess.hpp"

namespace chess
{
	class Terminal
	{
	public:

		void step();
		void wait_for_any_key();

		bool should_close() const;
		void set_board(const chess::Board& _board);





		Terminal(const char* _assetsDirectory, bool _step = false);
		~Terminal();

	private:
		Terminal(const Terminal&) = delete;
		Terminal(Terminal&&) = delete;


		int cw_;
		int ch_;
		bool should_close_ = false;
		bool step_ = false;
	};
};
