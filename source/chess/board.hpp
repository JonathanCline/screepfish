#pragma once

/** @file */

#include "board_base.hpp"

#include "piece.hpp"
#include "rating.hpp"
#include "bitboard.hpp"
#include "position.hpp"

#include "utility/number.hpp"
#include "utility/utility.hpp"

#include <jclib/type.h>
#include <jclib/type_traits.h>

#include <iosfwd>
#include <string>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>
#include <compare>
#include <ranges>
#include <algorithm>
#include <optional>
#include <array>
#include <span>
#include <random>

namespace chess
{
	enum class OutcomeType
	{
		none,
		draw,
		mate,
	};

	/**
	 * @brief Fufilled by types that can be used to track additional board info.
	*/
	template <typename T>
	concept cx_additional_board_info = requires(T& v, const T& cv, const BoardBase& _board, Move _move)
	{
		v.clear();
		v.sync(_board);
		v.move(_board, _move);
	};

	/**
	 * @brief Provides additional tracking for where pieces are attacking on a chess board.
	*/
	class BoardPieceAttackData
	{
	public:

		/**
		 * @brief Resets the attack data to an initial state.
		*/
		void clear();

		/**
		 * @brief Syncs the attack data with a new board.
		 * @param _board Board to sync with.
		*/
		void sync(const BoardBase& _board);

		/**
		 * @brief Updates the tracked data to reflect a played move.
		 * 
		 * Move legality is not checked.
		 * 
		 * @param _previousBoard The previous board state.
		 * @param _move Move to play.
		*/
		void move(const BoardBase& _previousBoard, Move _move);

		const BitBoard& get_black_direct_attacking() const
		{
			return this->battack_;
		};
		const BitBoard& get_white_direct_attacking() const
		{
			return this->wattack_;
		};
		
		const BitBoard& get_direct_attacking(Color _player) const
		{
			return (_player == Color::white)? this->get_white_direct_attacking() : this->get_black_direct_attacking();
		};


		BoardPieceAttackData() = default;

	private:

		/**
		 * @brief Where the white pieces are attacking.
		*/
		BitBoard wattack_;

		/**
		 * @brief Where the black pieces are attacking.
		*/
		BitBoard battack_;

	};

	namespace impl
	{
		/**
		 * @brief Helper type for holding onto additional board data.
		*/
		template <cx_additional_board_info... Ts>
		class BoardExtrasImpl
		{
		public:

			/**
			 * @brief Resets the extra data to an initial state.
			*/
			void clear()
			{
				(std::get<Ts>(this->extras_).clear(), ...);
			};

			/**
			 * @brief Syncs the extra data with a new board.
			 * @param _board Board to sync with.
			*/
			void sync(const BoardBase& _board)
			{
				(std::get<Ts>(this->extras_).sync(_board), ...);
			};

			/**
			 * @brief Updates the extra data to reflect a played move.
			 *
			 * Move legality is not checked.
			 *
			 * @param _previousBoard The previous board state.
			 * @param _move Move to play.
			*/
			void move(const BoardBase& _previousBoard, Move _move)
			{
				(std::get<Ts>(this->extras_).move(_previousBoard, _move), ...);
			};

			template <typename T>
			auto& get() { return std::get<T>(this->extras_); };
			template <typename T>
			const auto& get() const { return std::get<T>(this->extras_); };

			BoardExtrasImpl() = default;

		private:
			std::tuple<Ts...> extras_{};
		};

		template <>
		class BoardExtrasImpl<>
		{
		public:

			/**
			 * @brief Resets the extra data to an initial state.
			*/
			void clear()
			{
			};

			/**
			 * @brief Syncs the extra data with a new board.
			 * @param _board Board to sync with.
			*/
			void sync(const BoardBase& _board)
			{
			};

			/**
			 * @brief Updates the extra data to reflect a played move.
			 *
			 * Move legality is not checked.
			 *
			 * @param _previousBoard The previous board state.
			 * @param _move Move to play.
			*/
			void move(const BoardBase& _previousBoard, Move _move)
			{
			};

			BoardExtrasImpl() = default;

		private:
		};
	};

	/**
	 * @brief Additional board data storage / tracking.
	*/
	using BoardExtras = impl::BoardExtrasImpl
	<
		
	>;


	class Board;

	std::ostream& operator<<(std::ostream& _ostr, const Board& _value);

	/**
	 * @brief Represents a chess board with additional info tracked to hasten evaluation.
	*/
	class Board
	{
	private:
		friend std::ostream& operator<<(std::ostream& _ostr, const Board& _value);

		auto& board() { return this->board_; };
		auto& board() const { return this->board_; };
		auto& extra() { return this->extra_; };
		auto& extra() const { return this->extra_; };

	public:
		using iterator = typename BoardBase::iterator;
		using const_iterator = typename BoardBase::const_iterator;

		iterator begin() noexcept
		{
			return this->board().begin();
		};
		const_iterator begin() const noexcept
		{
			return this->board().begin();
		};
		iterator end() noexcept
		{
			return this->board().end();
		};
		const_iterator end() const noexcept
		{
			return this->board().end();
		};

	public:
		using piterator = typename BoardBase::piterator;
		using const_piterator = typename BoardBase::const_piterator;

		piterator pbegin() noexcept
		{
			return this->board().pbegin();
		};
		const_piterator pbegin() const noexcept
		{
			return this->board().pbegin();
		};
		piterator pend() noexcept
		{
			return this->board().pend();
		};
		const_piterator pend() const noexcept
		{
			return this->board().pend();
		};

		auto& pback()
		{
			return this->board().pback();
		};
		const auto& pback() const
		{
			return this->board().pback();
		};

		// Gets the king piece
		const BoardPiece& get_white_king() const
		{
			return this->board().get_white_king();
		};
		const BoardPiece& get_black_king() const
		{
			return this->board().get_black_king();
		};
		const BoardPiece& get_king(Color _color) const
		{
			return this->board().get_king(_color);
		};

		// find for pieces container
		piterator pfind(const Piece& _piece)
		{
			return this->board().pfind(_piece);
		};
		// find for pieces container
		const_piterator pfind(const Piece& _piece) const
		{
			return this->board().pfind(_piece);
		};

		// find for pieces container
		piterator pfind(PieceType _type, Color _color)
		{
			return this->pfind(Piece(_type, _color));
		};
		// find for pieces container
		const_piterator pfind(PieceType _type, Color _color) const
		{
			return this->pfind(Piece(_type, _color));
		};

		// find for pieces container
		piterator pfind(const Position& _pos)
		{
			return std::find(this->pbegin(), this->pend(), _pos);
		};
		// find for pieces container
		const_piterator pfind(const Position& _pos) const
		{
			return std::find(this->pbegin(), this->pend(), _pos);
		};

	public:

		auto find(Position _pos)
		{
			return this->board().find(_pos);
		};
		auto find(Position _pos) const
		{
			return this->board().find(_pos);
		};

		iterator find(Piece _piece)
		{
			return this->board().find(_piece);
		};
		const_iterator find(Piece _piece) const
		{
			return this->board().find(_piece);
		};

		iterator find(PieceType _piece, Color _color)
		{
			return this->find(Piece(_piece, _color));
		};
		const_iterator find(PieceType _piece, Color _color) const
		{
			return this->find(Piece(_piece, _color));
		};

		void sync() noexcept
		{
			this->extra().sync(this->board());
		};
		void clear() noexcept
		{
			this->extra().clear();
			this->board().clear();
		};

		void new_piece(Piece _piece, Position _pos)
		{
			return this->board().new_piece(_piece, _pos);
		};
		void new_piece(PieceType _piece, Color _color, Position _pos)
		{
			this->new_piece(Piece(_piece, _color), _pos);
		};

		Piece get(Position _pos) const
		{
			return this->board().get(_pos);
		};
		auto get(File _file, Rank _rank) const
		{
			return this->get(Position(_file, _rank));
		};


		/**
		 * @brief Gets the pieces that are on a particular file.
		 * @param _file File to get pieces on.
		 * @return Array of pieces, null piece means nothing is present.
		*/
		std::array<Piece, 8> pieces_on_file(File _file) const
		{
			return this->board().pieces_on_file(_file);
		};

		/**
		 * @brief Gets the pieces that are on a particular rank.
		 * @param _rank Rank to get pieces on.
		 * @return Array of pieces, null piece means nothing is present.
		*/
		std::array<Piece, 8> pieces_on_rank(Rank _rank) const
		{
			return this->board().pieces_on_rank(_rank);
		};

		/**
		 * @brief Gets the player who is to move.
		 * @return Color.
		*/
		Color get_toplay() const noexcept
		{
			return this->board().get_toplay();
		};

		void set_toplay(Color _toplay)
		{
			return this->board().set_toplay(_toplay);
		};

		bool has_enemy_piece(Position _pos, Color _myColor) const
		{
			return this->board().has_enemy_piece(_pos, _myColor);
		};
		bool has_friendy_piece(Position _pos, Color _myColor) const
		{
			return this->board().has_friendy_piece(_pos, _myColor);
		};
		bool has_piece(Position _pos) const
		{
			return this->board().has_piece(_pos);
		};
		bool has_enemy_piece_or_empty(Position _pos, Color _myColor) const
		{
			return this->board().has_enemy_piece_or_empty(_pos, _myColor);
		};
		bool is_empty(Position _pos) const
		{
			return this->board().is_empty(_pos);
		};

		void erase_piece(Position _position)
		{
			this->board().erase_piece(_position);
			this->sync();
		};

		void move(const Move& _move)
		{
			this->board().move(_move);
			this->extra().sync(this->board());
		};

		void move(const PieceMove& _move)
		{
			this->move(Move(_move));
		};
		void move(Position _from, Position _to)
		{
			this->move(Move(_from, _to));
		};


		bool has_enpassant_target() const noexcept
		{
			return this->board().has_enpassant_target();
		};
		Position enpassant_target() const noexcept
		{
			return this->board().enpassant_target();
		};
		void set_enpassant_target(Position _pos)
		{
			return this->board().set_enpassant_target(_pos);
		};

		bool get_castle_kingside_flag(Color _player) const
		{
			return this->board().get_castle_kingside_flag(_player);
		};
		bool get_castle_queenside_flag(Color _player) const
		{
			return this->board().get_castle_queenside_flag(_player);
		};

		void set_castle_kingside_flag(Color _player, bool _flag)
		{
			return this->board().set_castle_kingside_flag(_player, _flag);
		};
		void set_castle_queenside_flag(Color _player, bool _flag)
		{
			return this->board().set_castle_queenside_flag(_player, _flag);
		};

		uint16_t get_full_move_count() const
		{
			return this->board().get_full_move_count();
		};
		uint16_t get_half_move_count() const
		{
			return this->board().get_half_move_count();
		};

		void set_full_move_count(uint16_t _count)
		{
			return this->board().set_full_move_count(_count);
		};
		void set_half_move_count(uint16_t _count)
		{
			return this->board().set_half_move_count(_count);
		};

		BitBoard get_black_piece_bitboard() const
		{
			return this->board().get_black_piece_bitboard();
		};
		BitBoard get_white_piece_bitboard() const
		{
			return this->board().get_white_piece_bitboard();
		};

		const std::span<const BoardPiece> pieces() const
		{
			return this->board().pieces();
		};

		Move get_last_move() const
		{
			return this->board_.get_last_move();
		};

		auto* operator->() { return &this->board_; };
		auto* operator->() const { return &this->board_; };


		Board() = default;
	private:
		BoardBase board_;
		BoardExtras extra_;
	};


}