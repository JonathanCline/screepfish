#pragma once

/** @file */


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
	class Board;

	/**
	 * @brief Represents a chess board with only enough info to track game state.
	*/
	class BoardBase
	{
	private:

		using size_type = uint64_t;
		constexpr size_type toindex(const Position& _pos) const
		{
			return static_cast<size_type>(_pos);
		};
		constexpr size_type toindex(File _file, Rank _rank) const
		{
			return this->toindex(Position(_file, _rank));
		};

		constexpr Position topos(size_type _index) const
		{
			const auto _rank = Rank(_index & 0b0000'0111);
			const auto _file = File((_index & 0b0011'1000) >> 3);
			return Position(_file, _rank);
		};

		using container_type = std::array<Piece, 64>;

	public:
		using iterator = typename container_type::iterator;
		using const_iterator = typename container_type::const_iterator;

		iterator begin() noexcept
		{
			return this->pieces_by_pos_.begin();
		};
		const_iterator begin() const noexcept
		{
			return this->pieces_by_pos_.begin();
		};
		iterator end() noexcept
		{
			return this->pieces_by_pos_.end();
		};
		const_iterator end() const noexcept
		{
			return this->pieces_by_pos_.end();
		};

	private:

		// Pieces vec iterator access

		using pcontainer_type = std::array<BoardPiece, 32>;

		enum CastleBit : uint8_t
		{
			wking  = 0b0001,
			wqueen = 0b0010,
			bking  = 0b0100,
			bqueen = 0b1000
		};

	public:
		using piterator = typename pcontainer_type::iterator;
		using const_piterator = typename pcontainer_type::const_iterator;

		piterator pbegin() noexcept
		{
			return this->pieces_.begin();
		};
		const_piterator pbegin() const noexcept
		{
			return this->pieces_.begin();
		};
		piterator pend() noexcept
		{
			return std::find(this->pbegin(), this->pieces_.end(), PieceType::none);
		};
		const_piterator pend() const noexcept
		{
			return std::find(this->pbegin(), this->pieces_.end(), PieceType::none);
		};

		auto& pback()
		{
			return *(this->pend() - 1);
		};
		const auto& pback() const
		{
			return *(this->pend() - 1);
		};

		// Gets the king piece
		const BoardPiece& get_white_king() const
		{
			return this->pieces_.at(0);
		};
		const BoardPiece& get_black_king() const
		{
			return this->pieces_.at(1);
		};
		const BoardPiece& get_king(Color _color) const
		{
			if (_color == Color::white)
			{
				return this->get_white_king();
			}
			else
			{
				return this->get_black_king();
			};
		};

		// find for pieces container
		piterator pfind(const Piece& _piece)
		{
			if (_piece == Piece::black_king)
			{
				return this->pbegin() + 1;
			}
			else if (_piece == Piece::white_king)
			{
				return this->pbegin();
			};
			return std::find(this->pbegin(), this->pend(), _piece);
		};
		// find for pieces container
		const_piterator pfind(const Piece& _piece) const
		{
			if (_piece == Piece::black_king)
			{
				return this->pbegin() + 1;
			}
			else if (_piece == Piece::white_king)
			{
				return this->pbegin();
			};
			return std::find(this->pbegin(), this->pend(), _piece);
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

	private:

		void perase(piterator it)
		{
			const auto n = this->pend() - this->pbegin();

			if (it == this->pend() - 1)
			{
				this->pback() = BoardPiece();
			}
			else
			{
				const auto b = this->pback();
				this->pback() = BoardPiece();
				*it = b;
			};

			const auto n2 = this->pend() - this->pbegin();
			if (n2 != (n - 1))
			{
				abort();
			};
		};

		void erase(const Position& _pos, piterator pIt)
		{
			this->pieces_by_pos_.at(this->toindex(_pos)) = Piece{};
			if (pIt != this->pend())
			{
				if (pIt->color() == Color::white)
				{
					this->wpieces_.reset(_pos);
				}
				else
				{
					this->bpieces_.reset(_pos);
				};

				this->perase(pIt);
			};
		};
		void erase(const Position& _pos)
		{
			return this->erase(_pos, this->pfind(_pos));
		};


		void just_move_piece(Position _from, Position _to, piterator pIt)
		{
			assert(pIt != this->pend());

			auto& f = this->pieces_by_pos_.at(this->toindex(_from));
			auto& t = this->pieces_by_pos_.at(this->toindex(_to));

			// Remove old piece bit
			if (pIt->color() == Color::white)
			{
				this->wpieces_.reset(_from);
			}
			else
			{
				this->bpieces_.reset(_from);
			};

			// Check if destination position has a piece
			if (t)
			{
				const auto toIt = this->pfind(_to);

				// Remove captured piece bit
				if (toIt->color() == Color::white)
				{
					this->wpieces_.reset(_to);
				}
				else
				{
					this->bpieces_.reset(_to);
				};
				pIt->set_position(_to);
				this->perase(toIt);
			}
			else
			{
				pIt->set_position(_to);
			};

			// Set new piece bit
			if (pIt->color() == Color::white)
			{
				this->wpieces_.set(_to);
			}
			else
			{
				this->bpieces_.set(_to);
			};

			t = f;
			f = Piece{};
		};
		void just_move_piece(Position _from, Position _to)
		{
			const auto pIt = this->pfind(_from);
			assert(pIt != this->pend());
			return this->just_move_piece(_from, _to, pIt);
		};

	public:

		auto find(Position _pos)
		{
			return this->pieces_by_pos_.begin() + this->toindex(_pos);
		};
		auto find(Position _pos) const
		{
			return this->pieces_by_pos_.begin() + this->toindex(_pos);
		};

		iterator find(Piece _piece)
		{
			return std::ranges::find(this->pieces_by_pos_, _piece);
		};
		const_iterator find(Piece _piece) const
		{
			return std::ranges::find(this->pieces_by_pos_, _piece);
		};

		iterator find(PieceType _piece, Color _color)
		{
			return this->find(Piece(_piece, _color));
		};
		const_iterator find(PieceType _piece, Color _color) const
		{
			return this->find(Piece(_piece, _color));
		};



		void clear() noexcept
		{
			this->pieces_by_pos_.fill(Piece{});
			this->toplay_ = Color::white;
			this->castle_bits_ = CastleBit{};
			this->enpassant_target_.reset();
			this->fullmove_count_ = 1;
			this->halfmove_count_ = 0;

			this->pieces_.fill(BoardPiece{});
			this->bpieces_.reset();
			this->wpieces_.reset();

			this->last_moves_.fill(jc::null);
		};

		void new_piece(Piece _piece, Position _pos)
		{
			if (_piece == Piece::white_king && this->pieces_.size() > 1)
			{
				const auto o = this->pieces_.front();
				assert(o != Piece::white_king);
				this->new_piece(o, o.position());
				this->pieces_.front() = BoardPiece(_piece, _pos);
			}
			else if (_piece == Piece::black_king && this->pieces_.size() > 2)
			{
				const auto o = this->pieces_.at(1);
				assert(o != Piece::black_king);
				this->new_piece(o, o.position());
				this->pieces_.at(1) = BoardPiece(_piece, _pos);
			}
			else
			{
				*this->pend() = BoardPiece(_piece, _pos);
			};

			this->pieces_by_pos_.at(this->toindex(_pos)) = _piece;

			if (_piece.color() == Color::white)
			{
				this->wpieces_.set(_pos);
			}
			else
			{
				this->bpieces_.set(_pos);
			};
		};
		void new_piece(PieceType _piece, Color _color, Position _pos)
		{
			this->new_piece(Piece(_piece, _color), _pos);
		};

		Piece get(Position _pos) const
		{
			return this->pieces_by_pos_.at(this->toindex(_pos));
		};
		auto get(File _file, Rank _rank) const
		{
			return this->get((_file, _rank));
		};


		/**
		 * @brief Gets the pieces that are on a particular file.
		 * @param _file File to get pieces on.
		 * @return Array of pieces, null piece means nothing is present.
		*/
		std::array<Piece, 8> pieces_on_file(File _file) const
		{
			auto o = std::array<Piece, 8>{};
			auto it = o.begin();
			for (auto& _rank : ranks_v)
			{
				*it = this->pieces_by_pos_.at(this->toindex(_file, _rank));
				++it;
			};
			return o;
		};

		/**
		 * @brief Gets the pieces that are on a particular rank.
		 * @param _rank Rank to get pieces on.
		 * @return Array of pieces, null piece means nothing is present.
		*/
		std::array<Piece, 8> pieces_on_rank(Rank _rank) const
		{
			auto o = std::array<Piece, 8>{};
			auto it = o.begin();
			for (auto& _file : files_v)
			{
				*it = this->pieces_by_pos_.at(this->toindex(_file, _rank));
				++it;
			};
			return o;
		};

		/**
		 * @brief Gets the player who is to move.
		 * @return Color.
		*/
		Color get_toplay() const noexcept
		{
			return this->toplay_;
		};

		void set_toplay(Color _toplay)
		{
			this->toplay_ = _toplay;
		};


		bool has_enemy_piece(Position _pos, Color _myColor) const
		{
			if (const auto p = this->get(_pos); p)
			{
				return p.color() != _myColor;
			}
			else
			{
				return false;
			};
		};
		bool has_friendy_piece(Position _pos, Color _myColor) const
		{
			if (const auto p = this->get(_pos); p)
			{
				return p.color() == _myColor;
			}
			else
			{
				return false;
			};
		};
		bool has_piece(Position _pos) const
		{
			return (bool)this->get(_pos);
		};
		bool has_enemy_piece_or_empty(Position _pos, Color _myColor) const
		{
			if (const auto p = this->get(_pos); p)
			{
				return p.color() != _myColor;
			}
			else
			{
				return true;
			};
		};
		bool is_empty(Position _pos) const
		{
			return !this->has_piece(_pos);
		};


		void erase_piece(Position _position)
		{
			this->erase(_position);
		};

		void move(const Move& _move);

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
			return this->enpassant_target_.has_value();
		};
		Position enpassant_target() const noexcept
		{
			return this->enpassant_target_.value();
		};
		void set_enpassant_target(Position _pos)
		{
			this->enpassant_target_ = _pos;
		};

		bool get_castle_kingside_flag(Color _player) const
		{
			if (_player == Color::white)
			{
				return this->castle_bits_ & CastleBit::wking;
			}
			else
			{
				return this->castle_bits_ & CastleBit::bking;
			};
		};
		bool get_castle_queenside_flag(Color _player) const
		{
			if (_player == Color::white)
			{
				return this->castle_bits_ & CastleBit::wqueen;
			}
			else
			{
				return this->castle_bits_ & CastleBit::bqueen;
			};
		};

	private:

		void reset_castle_flag(CastleBit _bits)
		{
			this->castle_bits_ = CastleBit(this->castle_bits_ & ~_bits);
		};
		void set_castle_flag(CastleBit _bits)
		{
			this->castle_bits_ = CastleBit(this->castle_bits_ | _bits);
		};
		void set_castle_flag(CastleBit _bits, bool _state)
		{
			if (_state)
			{
				this->set_castle_flag(_bits);
			}
			else
			{
				this->reset_castle_flag(_bits);
			};
		};

	public:

		void set_castle_kingside_flag(Color _player, bool _flag)
		{
			if (_player == Color::white)
			{
				this->set_castle_flag(CastleBit::wking, _flag);
			}
			else
			{
				this->set_castle_flag(CastleBit::bking, _flag);
			};
		};
		void set_castle_queenside_flag(Color _player, bool _flag)
		{
			if (_player == Color::white)
			{
				this->set_castle_flag(CastleBit::wqueen, _flag);
			}
			else
			{
				this->set_castle_flag(CastleBit::bqueen, _flag);
			};
		};

		uint16_t get_full_move_count() const
		{
			return this->fullmove_count_;
		};
		uint16_t get_half_move_count() const
		{
			return this->halfmove_count_;
		};

		void set_full_move_count(uint16_t _count)
		{
			this->fullmove_count_ = _count;
		};
		void set_half_move_count(uint16_t _count)
		{
			this->halfmove_count_ = _count;
		};

		BitBoard get_black_piece_bitboard() const
		{
			return this->bpieces_;
		};
		BitBoard get_white_piece_bitboard() const
		{
			return this->wpieces_;
		};

	private:
		
		/**
		 * @brief Sets a move as the last played move.
		 * @param _move Last played move.
		*/
		void set_last_move(const Move& _move)
		{
			auto& _storage = this->last_moves_;
			std::shift_right(_storage.begin(), _storage.end(), 1);
			_storage.front() = _move;
		};

		void set_previous_board_hash(const Board& _board);

	public:

		/**
		 * @brief Gets the last move that was played.
		 * @return The last move played, or a null move if no moves have been played.
		*/
		Move get_last_move() const
		{
			return this->last_moves_.front();
		};

		bool is_repeated_move(Move _move) const;
		bool is_last_move_repeated_move() const;





		const std::span<const BoardPiece> pieces() const
		{
			return std::span<const BoardPiece>(this->pbegin(), this->pend());
		};
		

		friend std::ostream& operator<<(std::ostream& _ostr, const BoardBase& _value);

		BoardBase() = default;

	private:
		//container_type pieces_;
		std::array<Piece, 64> pieces_by_pos_{};
		std::array<BoardPiece, 32> pieces_{};

		BitBoard bpieces_;
		BitBoard wpieces_;

		/**
		 * @brief Holds the last move that was played on the board.
		*/
		std::array<Move, 5> last_moves_{};
		//std::array<uint32_t, 6> last_board_hashes_{};


		std::optional<Position> enpassant_target_;

		/**
		 * @brief Half-moves since the last piece capture or pawn advance.
		*/
		uint16_t halfmove_count_ = 0;

		/**
		 * @brief Full-moves played, increments every time black plays.
		*/
		uint16_t fullmove_count_ = 1;

		// Castle tracking
		CastleBit castle_bits_ = CastleBit{};

		/**
		 * @brief Whos turn it is to play.
		*/
		Color toplay_ = Color::white;

	};

	constexpr static auto p0 = sizeof(BoardBase);

}