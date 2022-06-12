#pragma once

/** @file */

#include "chess/rating.hpp"

#include "chess/chess.hpp"

#include <span>
#include <array>
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

		template <size_t N>
		explicit MoveBuffer(std::array<Move, N>& _buffer) :
			MoveBuffer(_buffer.data(), _buffer.data() + N)
		{};

	private:
		chess::Move* at_;
		chess::Move* end_;
	};

	struct RatedMove : public chess::Move
	{
	public:

		constexpr Rating rating() const noexcept { return this->rating_; };

		constexpr auto operator<=>(const RatedMove& rhs) const
		{
			return this->rating() <=> rhs.rating();
		};

		constexpr RatedMove() = default;
		constexpr RatedMove(Move _move, Rating _rating) :
			chess::Move(_move), rating_(_rating)
		{};
		constexpr RatedMove(PieceMove _move, Rating _rating) :
			RatedMove(Move(_move), _rating)
		{};
		constexpr RatedMove(PieceMove _move, PieceType _promotion, Rating _rating) :
			RatedMove(Move(_move, _promotion), _rating)
		{};
		constexpr RatedMove(Position _from, Position _to, Rating _rating) :
			RatedMove(Move(_from, _to), _rating)
		{};
		constexpr RatedMove(Position _from, Position _to, PieceType _promotion, Rating _rating) :
			RatedMove(Move(_from, _to, _promotion), _rating)
		{};

	private:
		chess::Rating rating_;
	};



	void get_piece_attacks_with_pawn(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer);
	void get_piece_attacks_with_knight(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer);
	void get_piece_attacks_with_bishop(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer);
	void get_piece_attacks_with_rook(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer);
	void get_piece_attacks_with_queen(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer);
	void get_piece_attacks_with_king(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece, MoveBuffer& _buffer);
	
	void get_piece_attacked_from_moves(const chess::Board& _board, const chess::BoardPiece& _piece, MoveBuffer& _buffer, bool _inCheck = false);



	bool is_piece_attacked_by_pawn(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	bool is_piece_attacked_by_knight(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	bool is_piece_attacked_by_bishop(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	bool is_piece_attacked_by_rook(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	bool is_piece_attacked_by_queen(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	bool is_piece_attacked_by_king(const chess::Board& _board, const chess::BoardPiece& _piece, const chess::BoardPiece& _byPiece);
	bool is_piece_attacked(const chess::Board& _board, const chess::BoardPiece& _piece, bool _inCheck = false);

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
	bool is_bishop_blocked(const Board& _board, Position _pos, Color _color);
	


	/**
	 * @brief Checks if a player is in check for a given board position.
	 * @param _board Current board position.
	 * @param _forPlayer Player to see if is in check.
	 * @return True if player given is in check, false otherwise.
	*/
	bool is_check(const chess::Board& _board, const chess::Color _forPlayer);

	/**
	 * @brief Checks if a player is in checkmate for a given board position.
	 * @param _board Current board position.
	 * @param _forPlayer Player to see if is in checkmate.
	 * @return True if player given is in checkmate, false otherwise.
	*/
	bool is_checkmate(const chess::Board& _board, const chess::Color _forPlayer);



	/**
	 * @brief Calculates a quick rating for a board based solely on the current position.
	 *
	 * This should be used to quickly calculate a basic value to assign as a rating, but shouldn't
	 * be used to determine anything more than instantaneous position rating.
	 * 
	 * @param _board Board to rate.
	 * @param _forPlayer Player to rate the board for.
	 * @return Rating for the given player.
	*/
	Rating quick_rate(const chess::Board& _board, chess::Color _forPlayer);

	/**
	 * @brief Calculates a quick rating for a board based solely on the current position.
	 *
	 * This should be used to quickly calculate a basic value to assign as a rating, but shouldn't
	 * be used to determine anything more than instantaneous position rating.
	 *
	 * @param _board Board to rate.
	 * @return Absolute board rating.
	*/
	AbsoluteRating quick_rate(const chess::Board& _board);

};