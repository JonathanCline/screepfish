#pragma once

/** @file */

#include "chess/chess.hpp"

#include <vector>

namespace sch
{
	struct MoveBuffer;

	struct RatedMove
	{
		constexpr auto operator<=>(const RatedMove& rhs) const
		{
			return this->rating <=> rhs.rating;
		};
		chess::Move move;
		chess::Rating rating;
	};

	class ScreepFish : public chess::IChessEngine
	{
	private:

		bool is_piece_attacked_by_pawn(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);


		bool is_piece_attacked(const chess::Board& _board, const chess::BoardPiece& _piece);

		void get_pawn_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer);
		void get_rook_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer);
		void get_knight_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer);
		void get_king_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer);
		void get_bishop_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer);
		void get_queen_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer);
		void get_piece_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer);

		bool is_check(const chess::Board& _board, const chess::Color _forPlayer);
		std::vector<chess::Move> get_moves(const chess::Board& _board, const chess::Color _forPlayer);
		bool is_checkmate(const chess::Board& _board, const chess::Color _forPlayer);

		chess::Rating rate_board(const chess::Board& _board, chess::Color _forPlayer);
		chess::Rating rate_move(const chess::Board& _board, const chess::Move& _move, chess::Color _forPlayer);
		
		RatedMove best_move_search(const chess::Board& _board, chess::Color _forPlayer, int _depth2);
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