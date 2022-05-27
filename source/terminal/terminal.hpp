#pragma once

/** @file */

#include "chess/chess.hpp"

namespace chess
{
	class Terminal
	{
	public:

		void wait_for_any_key();

		bool should_close() const;
		void set_board(const chess::Board& _board);





		Terminal(const char* _assetsDirectory);
		~Terminal();

	private:
		Terminal(const Terminal&) = delete;
		Terminal(Terminal&&) = delete;


		int cw_;
		int ch_;
		bool should_close_ = false;
	};
};
