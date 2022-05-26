#pragma once

/** @file */

#include "chess/chess.hpp"
#include "chess/move_tree.hpp"

namespace sch
{
	class ScreepFish : public chess::IChessEngine
	{
	private:

		chess::MoveTree build_move_tree(const chess::Board& _board, chess::Color _forPlayer, int _depth);

		chess::Move best_move(const chess::Board& _board, chess::Color _forPlayer, int _depth);

		struct BoardCache
		{
			size_t hash = 0;

			bool is_bcheck = false;
			bool is_bcheckmate = false;
			bool is_wcheck = false;
			bool is_wcheckmate = false;

			BoardCache() = default;
		};

		std::vector<BoardCache> cache_{};

		void cache(const chess::Board& _board);
		const BoardCache* get_cached(size_t _hash);

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