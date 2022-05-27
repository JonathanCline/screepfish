#pragma once

/** @file */

#include "chess/chess.hpp"

#include <span>
#include <cassert>

namespace chess
{
	struct MoveBuffer
	{
	public:

		Move* head() const { return this->at_; };

		void write(Move _move)
		{
			assert(this->at_ != this->end_);
			*this->at_ = std::move(_move);
			++this->at_;
		};
		void write(chess::Position _from, chess::Position _to)
		{
			return this->write(chess::Move(_from, _to));
		};

		MoveBuffer(chess::Move* _at, chess::Move* _end) :
			at_(_at), end_(_end)
		{
			assert(_at && _end && _at <= _end);
		};

	private:
		chess::Move* at_;
		chess::Move* end_;
	};

	struct RatedMove
	{
		constexpr auto operator<=>(const RatedMove& rhs) const
		{
			return this->rating <=> rhs.rating;
		};

		chess::Move move;
		chess::Rating rating;
	};


	bool is_piece_attacked_by_pawn_old(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	bool is_piece_attacked_by_pawn(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	
	bool is_piece_attacked_by_knight(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	bool is_piece_attacked_by_bishop(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	bool is_piece_attacked_by_rook(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	bool is_piece_attacked_by_queen(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	bool is_piece_attacked_by_king(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);

	bool is_piece_attacked_old(const chess::Board& _board, const chess::BoardPiece& _piece);
	bool is_piece_attacked(const chess::Board& _board, const chess::BoardPiece& _piece);

	void get_pawn_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck = false);
	void get_rook_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck = false);
	void get_knight_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck = false);
	void get_king_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck = false);
	void get_bishop_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck = false);
	void get_queen_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck = false);
	void get_piece_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, const bool _isCheck = false);

	void get_moves(const chess::Board& _board, const chess::Color _forPlayer, MoveBuffer& _buffer, const bool _isCheck = false);


	bool can_castle_kingside(const chess::Board& _board, chess::Color _player);
	bool can_castle_queenside(const chess::Board& _board, chess::Color _player);



	/**
	 * @brief Gets the valid positions surrounding a position.
	 * @param _pos Position to get surrounding positions of.
	 * @return Span of positions.
	*/
	std::span<const Position> get_surrounding_positions(Position _pos);

	/**
	 * @brief Gets the valid positions that a rook can move towards surrounding a position.
	 * @param _pos Position to get surrounding positions of.
	 * @return Span of positions.
	*/
	std::span<const Position> get_surrounding_positions_for_rook(Position _pos);


	bool is_queen_blocked(const Board& _board, const Position _pos, Color _color);
	bool is_rook_blocked(const Board& _board, const Position _pos, Color _color);

	


	bool is_check(const chess::Board& _board, const chess::Color _forPlayer);
	bool is_checkmate(const chess::Board& _board, const chess::Color _forPlayer);

	chess::Rating rate_board(const chess::Board& _board, chess::Color _forPlayer);
	//chess::Rating rate_move(const chess::Board& _board, const chess::Move& _move, chess::Color _forPlayer, const bool _isCheck = false);

};