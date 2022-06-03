#pragma once

/** @file */

#include "bitboard.hpp"
#include "position.hpp"

#include "utility/number.hpp"
#include "utility/utility.hpp"

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


namespace chess
{



	/**
	 * @brief Named piece / square colors.
	*/
	enum class Color : bool
	{
		black,
		white
	};

	constexpr inline auto colors_v = std::array
	{
		Color::black,
		Color::white
	};


	/**
	 * @brief Inverts the given color.
	 * 
	 * white -> black
	 * black -> white
	 * 
	 * @param rhs Color to invert.
	 * @return Inverted (opposite) color.
	*/
	constexpr Color operator!(Color rhs) noexcept
	{
		return Color(!jc::to_underlying(rhs));
	};



	constexpr Color square_color(Position _pos)
	{
		const auto a = static_cast<size_t>(_pos);
		const auto b = (((int)_pos.rank() % 2) == 0);
		const auto c = (((int)_pos.file() % 2) == 0);
		return (b ^ c) ? Color::white : Color::black;
	};




	enum class OutcomeType
	{
		none,
		draw,
		mate,
	};

	using Rating = float;

	












	/**
	 * @brief Represents a movement of a piece by a player.
	*/
	class PieceMove
	{
	public:

		/**
		 * @brief Gets the position the piece moved from.
		*/
		constexpr Position from() const noexcept { return this->from_; };

		/**
		 * @brief Gets the position the piece moved to.
		*/
		constexpr Position to() const noexcept { return this->to_; };



		constexpr PieceMove() = default;
		constexpr PieceMove(Position _from, Position _to) :
			from_(_from), to_(_to)
		{};

	private:

		/**
		 * @brief The position the piece moved from.
		*/
		Position from_;

		/**
		 * @brief The position the piece moved to.
		*/
		Position to_;
	};

	/**
	 * @brief Parses a move value from a string.
	 * 
	 * Format is assumed like "a1b2" where 'a1' is from pos and 'b2' is to pos.
	 *
	 * @param _str String to parse.
	 * @param _value Where to write the parsed value.
	 * @return String with parsed characters removed.
	*/
	constexpr std::string_view fromstr(std::string_view _str, PieceMove& _value)
	{
		// Parse individual positions.
		Position _from, _to;
		_str = fromstr(_str, _from);
		_str = fromstr(_str, _to);

		// Write parsed move.
		_value = PieceMove(_from, _to);
		return _str;
	};

	std::ostream& operator<<(std::ostream& _ostr, const PieceMove& _value);



	/**
	 * @brief Types of chess pieces.
	*/
	enum class PieceType : uint8_t
	{
		none = 0, // maybe
		pawn = 1,
		knight = 2,
		bishop = 3,
		rook = 4,
		queen = 5,
		king = 6
	};

	constexpr inline auto piece_types_v = std::array
	{
		PieceType::none,
		PieceType::pawn,
		PieceType::knight,
		PieceType::bishop,
		PieceType::rook,
		PieceType::queen,
		PieceType::king,
	};


	class Move : public PieceMove
	{
	public:

		constexpr PieceType promotion() const
		{
			return this->promotion_;
		};

		constexpr Move() = default;
		constexpr Move(PieceMove _move) :
			PieceMove(_move), promotion_(PieceType::queen)
		{};
		constexpr Move(PieceMove _move, PieceType _promotion) :
			PieceMove(_move), promotion_(_promotion)
		{};
		constexpr Move(Position _from, Position _to, PieceType _promotion) :
			PieceMove(_from, _to),
			promotion_{ _promotion }
		{};
		constexpr Move(Position _from, Position _to) :
			Move(_from, _to, PieceType{})
		{};
	
	private:
		PieceType promotion_;
	};

	/**
	 * @brief Parses a move value from a string.
	 *
	 * Format is assumed like "a1b2" where 'a1' is from pos and 'b2' is to pos.
	 *
	 * @param _str String to parse.
	 * @param _value Where to write the parsed value.
	 * @return String with parsed characters removed.
	*/
	constexpr std::string_view fromstr(std::string_view _str, Move& _value)
	{
		// Parse individual positions.
		Position _from, _to;
		_str = fromstr(_str, _from);
		_str = fromstr(_str, _to);

		auto _promotion = PieceType::none;
		if (!_str.empty())
		{
			const auto c = _str.front();
			switch(c)
			{
			case 'q':
				_promotion = PieceType::queen;
				break;
			case 'n':
				_promotion = PieceType::knight;
				break;
			case 'r':
				_promotion = PieceType::rook;
				break;
			case 'b':
				_promotion = PieceType::bishop;
				break;
			default:
				break;
			};
			if (_promotion != PieceType::none)
			{
				_str.remove_prefix(1);
			};
		};

		// Write parsed move.
		_value = Move(_from, _to, _promotion);
		return _str;
	};

	std::ostream& operator<<(std::ostream& _ostr, const Move& _value);







	/**
	 * @brief Represents a chess piece with defined color.
	*/
	class Piece
	{
	public:

		/**
		 * @brief Exposes the piece type enum values for use
		*/
		using enum PieceType;

		enum class PieceE : uint8_t
		{
			black_pawn   = 0b0010,
			black_knight = 0b0100,
			black_bishop = 0b0110,
			black_rook	 = 0b1000,
			black_queen  = 0b1010,
			black_king   = 0b1100,

			white_pawn		= 0b0011,
			white_knight	= 0b0101,
			white_bishop	= 0b0111,
			white_rook		= 0b1001,
			white_queen		= 0b1011,
			white_king		= 0b1101,
		};
		using enum PieceE;

		/**
		 * @brief Types of chess pieces.
		*/
		using Type = PieceType;

		/**
		 * @brief Gets the type of the piece.
		 * @return Chess piece type.
		*/
		constexpr Type type() const noexcept
		{
			return Type((this->piece_ & 0b1111'1110) >> 1);
		};

		/**
		 * @brief Checks that the piece type is valid.
		 * @return True if valid, false otherwise.
		*/
		constexpr explicit operator bool() const noexcept
		{
			return this->type() != Type::none;
		};

		/**
		 * @brief Gets the color of the piece.
		 * @return Chess piece color.
		*/
		constexpr Color color() const noexcept
		{
			return Color(this->piece_ & 0x1);
		};

		constexpr bool is_white() const
		{
			return this->piece_ & 0x1;
		};

		constexpr explicit operator uint8_t() const noexcept { return this->piece_; };
		constexpr operator PieceE() const noexcept { return PieceE(this->piece_); };


		// Compares color AND piece type
		constexpr bool operator==(const Piece& rhs) const noexcept = default;
		// Compares color AND piece type
		constexpr bool operator!=(const Piece& rhs) const noexcept = default;

		// Compares piece type
		friend inline constexpr bool operator==(const Piece& lhs, const PieceType& rhs) noexcept
		{
			return lhs.type() == rhs;
		};
		// Compares piece type
		friend inline constexpr bool operator==(const PieceType& lhs, const Piece& rhs) noexcept
		{
			return lhs == rhs.type();
		};

		// Compares piece type
		friend inline constexpr bool operator!=(const Piece& lhs, const PieceType& rhs) noexcept
		{
			return lhs.type() != rhs;
		};
		// Compares piece type
		friend inline constexpr bool operator!=(const PieceType& lhs, const Piece& rhs) noexcept
		{
			return lhs != rhs.type();
		};





		constexpr Piece() noexcept :
			piece_(0)
		{};
		constexpr Piece(Type _type, Color _color) noexcept :
			piece_((jc::to_underlying(_type) << 1) | ((_color == Color::white)? 0b1 : 0b0))
		{
			if (_type == Type::none) { abort(); };
		};
		
		constexpr Piece& operator=(PieceType _type)
		{
			if (_type == PieceType::none)
			{
				this->piece_ = 0;
			}
			else
			{
				this->piece_ = (jc::to_underlying(_type) << 1) | (this->piece_ & 0x1);
			};
			return *this;
		};

	private:

		/**
		 * @brief The piece type with color value.
		*/
		uint8_t piece_;

	};



	/**
	 * @brief Represents a piece actually on a chess board.
	*/
	class BoardPiece : public Piece
	{
	public:
		constexpr Position position() const
		{
			return this->pos_;
		};
		constexpr void set_position(Position _pos) noexcept
		{
			this->pos_ = _pos;
		};

		constexpr Rank rank() const noexcept { return this->position().rank(); };
		constexpr File file() const noexcept { return this->position().file(); };

		friend constexpr inline bool operator==(const BoardPiece& lhs, const Position& rhs) noexcept
		{
			return lhs.position() == rhs;
		};
		friend constexpr inline bool operator==(const Position& lhs, const BoardPiece& rhs) noexcept
		{
			return lhs == rhs.position();
		};
		friend constexpr inline bool operator!=(const BoardPiece& lhs, const Position& rhs) noexcept
		{
			return lhs.position() != rhs;
		};
		friend constexpr inline bool operator!=(const Position& lhs, const BoardPiece& rhs) noexcept
		{
			return lhs != rhs.position();
		};

		constexpr void promote(PieceType _type) noexcept
		{
			*this = BoardPiece(_type, this->color(), this->position());
		};

		constexpr BoardPiece() :
			Piece(),
			pos_{}
		{}
		constexpr BoardPiece(Piece _piece, Position _pos) noexcept :
			Piece(_piece), pos_(_pos)
		{};
		constexpr BoardPiece(Type _type, Color _color, Position _pos) noexcept :
			BoardPiece(Piece(_type, _color), _pos)
		{};

		constexpr BoardPiece& operator=(Piece _piece) noexcept
		{
			Piece::operator=(_piece);
			return *this;
		};
		constexpr BoardPiece& operator=(PieceType _piece) noexcept
		{
			Piece::operator=(_piece);
			return *this;
		};

	private:
		Position pos_{};
	};

	/**
	 * @brief Represents a chess board by tracking the pieces directly.
	*/
	class Board
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

		// find for pieces container
		piterator pfind(const Piece& _piece)
		{
			return std::find(this->pbegin(), this->pend(), _piece);
		};
		// find for pieces container
		const_piterator pfind(const Piece& _piece) const
		{
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
			this->bcastle_king_ = false;
			this->bcastle_queen_ = false;
			this->wcastle_king_ = false;
			this->wcastle_queen_ = false;
			this->enpassant_target_.reset();
			this->fullmove_count_ = 1;
			this->halfmove_count_ = 0;

			this->pieces_.fill(BoardPiece{});
			this->bpieces_.reset();
			this->wpieces_.reset();
		};

		void new_piece(Piece _piece, Position _pos)
		{
			this->pieces_by_pos_.at(this->toindex(_pos)) = _piece;
			*this->pend() = BoardPiece(_piece, _pos);

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
		void move_piece(Position _fromPos, Position _toPos, PieceType _promotion = PieceType::none)
		{
			const auto _oldEnpassantTarget = this->enpassant_target_;
			this->enpassant_target_.reset();

			{
				const auto _from = this->get(_fromPos);
				const auto _to = this->get(_toPos);

				// Castling move handling
				if (_from == PieceType::king)
				{
					if (_from.color() == Color::white && _fromPos == (File::e, Rank::r1))
					{
						if (_toPos == (File::c, Rank::r1))
						{
							assert(this->wcastle_queen_);
							this->just_move_piece((File::a, Rank::r1), (File::d, Rank::r1));
						}
						else if (_toPos == (File::g, Rank::r1))
						{
							assert(this->wcastle_king_);
							this->just_move_piece((File::h, Rank::r1), (File::f, Rank::r1));
						};
					}
					else if (_from.color() == Color::black && _fromPos == (File::e, Rank::r8))
					{
						if (_toPos == (File::c, Rank::r8))
						{
							assert(this->bcastle_queen_);
							this->just_move_piece((File::a, Rank::r8), (File::d, Rank::r8));
						}
						else if (_toPos == (File::g, Rank::r8))
						{
							assert(this->bcastle_king_);
							this->just_move_piece((File::h, Rank::r8), (File::f, Rank::r8));
						};
					};
				}
				else if (_from.type() == PieceType::pawn)
				{
					if (_from.color() == Color::white)
					{
						if (_fromPos.rank() == Rank::r2 &&
							_toPos.rank() == Rank::r4)
						{
							this->enpassant_target_ = Position(_fromPos.file(), Rank::r3);
						};
					}
					else
					{
						if (_fromPos.rank() == Rank::r7 &&
							_toPos.rank() == Rank::r5)
						{
							this->enpassant_target_ = Position(_fromPos.file(), Rank::r6);
						};
					};
				};

				// Castling flag checking behavior
				if (_from == PieceType::king)
				{
					if (_from.color() == Color::white)
					{
						this->wcastle_king_ = false;
						this->wcastle_queen_ = false;
					}
					else
					{
						this->bcastle_king_ = false;
						this->bcastle_queen_ = false;
					};
				}
				else if (_from == PieceType::rook)
				{
					if (_from.color() == Color::white)
					{
						if (_fromPos == (File::h, Rank::r1))
						{
							this->wcastle_king_ = false;
						}
						else if (_fromPos == (File::a, Rank::r1))
						{
							this->wcastle_queen_ = false;
						};
					}
					else
					{
						if (_fromPos == (File::h, Rank::r8))
						{
							this->bcastle_king_ = false;
						}
						else if (_fromPos == (File::a, Rank::r8))
						{
							this->bcastle_queen_ = false;
						};
					};
				};

				// Half move handling (fifty move rule)
				if (_to || _from == Piece::pawn)
				{
					// Reset halfmove counter
					this->halfmove_count_ = 0;
				}
				else
				{
					// Increment halfmove counter
					++this->halfmove_count_;
				};
			};

			auto it = this->find(_fromPos);
			assert(it != this->end());

			if (_promotion != PieceType::none && *it == PieceType::pawn)
			{
				this->pieces_by_pos_.at(toindex(_fromPos)) = _promotion;
				*this->pfind(_fromPos) = _promotion;
			};

			if (_oldEnpassantTarget && *_oldEnpassantTarget == _toPos)
			{
				if (_toPos.rank() == Rank::r6)
				{
					this->erase((_toPos.file(), Rank::r5));
				}
				else
				{
					this->erase((_toPos.file(), Rank::r4));
				};
			};

			// Set new piece position
			this->just_move_piece(_fromPos, _toPos);

			// Increment move counter
			if (this->toplay_ == Color::black)
			{
				this->fullmove_count_ += 1;
			};
			
			// Swap to play flag
			this->toplay_ = !this->toplay_;
		};

		void move(const PieceMove& _move)
		{
			this->move_piece(_move.from(), _move.to());
		};
		void move(const Move& _move)
		{
			this->move_piece(_move.from(), _move.to(), _move.promotion());
		};
		void move(Position _from, Position _to)
		{
			this->move_piece(_from, _to);
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
				return this->wcastle_king_;
			}
			else
			{
				return this->bcastle_king_;
			};
		};
		bool get_castle_queenside_flag(Color _player) const
		{
			if (_player == Color::white)
			{
				return this->wcastle_queen_;
			}
			else
			{
				return this->bcastle_queen_;
			};
		};
		void set_castle_kingside_flag(Color _player, bool _flag)
		{
			if (_player == Color::white)
			{
				this->wcastle_king_ = _flag;
			}
			else
			{
				this->bcastle_king_ = _flag;
			};
		};
		void set_castle_queenside_flag(Color _player, bool _flag)
		{
			if (_player == Color::white)
			{
				this->wcastle_queen_ = _flag;
			}
			else
			{
				this->bcastle_queen_ = _flag;
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



		const std::span<const BoardPiece> pieces() const
		{
			return std::span<const BoardPiece>(this->pbegin(), this->pend());
		};


		friend std::ostream& operator<<(std::ostream& _ostr, const Board& _value);

		Board() = default;

	private:
		//container_type pieces_;
		std::array<Piece, 64> pieces_by_pos_{};
		std::array<BoardPiece, 32> pieces_{};
		std::optional<Position> enpassant_target_;
		
		BitBoard bpieces_;
		BitBoard wpieces_;
		
		/**
		 * @brief Whos turn it is to play.
		*/
		Color toplay_ = Color::white;

		/**
		 * @brief Half-moves since the last piece capture or pawn advance.
		*/
		uint16_t halfmove_count_ = 0;

		/**
		 * @brief Full-moves played, increments every time black plays.
		*/
		uint16_t fullmove_count_ = 1;

		// Castle tracking
		bool bcastle_king_ = false;
		bool bcastle_queen_ = false;
		bool wcastle_king_ = false;
		bool wcastle_queen_ = false;

	};
	
	


	struct ZobristHashTable
	{
		std::array<std::array<size_t, 12>, 64> table{};
		size_t black_to_move{};
	};

	constexpr auto zobrist_hash_subindex(PieceType p, Color c)
	{
		size_t _subindex = 0;
		if (c == Color::white) { _subindex = 1; };
		_subindex |= static_cast<size_t>(jc::to_underlying(p) - 1) << 1;
		return _subindex;
	};

	template <typename T = uint64_t>
	constexpr T pseudorand(T _value,
		const T _largeA = 125361361361603, const T _largeB = 995995959582, 
		const T _mod = 10000)
	{
		return (static_cast<T>(_largeA * _value) + _largeB) % _mod;
	};

	constexpr auto zobrist_hash_index(File f, Rank r)
	{
		size_t _hash = 0;
		_hash |= ((jc::to_underlying(f) << 3) | jc::to_underlying(r));
		return _hash;
	};

	consteval auto zobrist_hash_table()
	{
		auto _table = ZobristHashTable{};
		size_t _pseudoRand = 13136;
		for (auto& f : files_v)
		{
			for (auto& r : ranks_v)
			{
				const auto i = zobrist_hash_index(f, r);
				for (auto& p : piece_types_v)
				{
					if (p == PieceType::none)
						continue;
					
					for (auto& c : colors_v)
					{
						const auto j = zobrist_hash_subindex(p, c);
						_pseudoRand = pseudorand(_pseudoRand);
						const auto _hash = _pseudoRand;
						_table.table[i][j] = _hash;
					};
				};
			};
		};

		_table.black_to_move = pseudorand(_pseudoRand);
		return _table;
	};

	constexpr inline auto zobrist_hash_lookup_table_v =
		zobrist_hash_table();

	/**
	 * @brief Calculates a zobrist hash for a board.
	 * @param _board Chess board.
	 * @param _blackToMove Whether or not it is blacks turn to move.
	 * @return Zobrist hash.
	*/
	inline auto hash(const Board& _board, bool _blackToMove)
	{
		size_t _hash = 0;
		if (_blackToMove)
		{
			_hash ^= zobrist_hash_lookup_table_v.black_to_move;
		};

		for (auto& f : files_v)
		{
			for (auto& r : ranks_v)
			{
				const auto p = _board.get(f, r);
				if (p)
				{
					const auto i = zobrist_hash_index(f, r);
					const auto j = zobrist_hash_subindex(p.type(), p.color());
					_hash ^= zobrist_hash_lookup_table_v.table[i][j];
				};
			};
		};

		return _hash;
	};





	/**
	 * @brief Resets a board to the standard chess starting positions.
	 * @param _board The board to reset.
	 * @return The given board, now reset.
	*/
	inline Board& reset_board(Board& _board)
	{
		// Clear the board so we can begin anew.
		_board.clear();

		// Kings in front as they are commonly needed
		_board.new_piece(PieceType::king,	Color::white, (File::e, Rank::r1));
		_board.new_piece(PieceType::king,	Color::black, (File::e, Rank::r8));



		// Set black positions
		
		// Back row
		_board.new_piece(PieceType::rook,	Color::black, (File::a, Rank::r8));
		_board.new_piece(PieceType::knight, Color::black, (File::b, Rank::r8));
		_board.new_piece(PieceType::bishop, Color::black, (File::c, Rank::r8));
		_board.new_piece(PieceType::queen,	Color::black, (File::d, Rank::r8));
		_board.new_piece(PieceType::bishop, Color::black, (File::f, Rank::r8));
		_board.new_piece(PieceType::rook,	Color::black, (File::h, Rank::r8));
		_board.new_piece(PieceType::knight, Color::black, (File::g, Rank::r8));

		// Castle flags
		_board.set_castle_kingside_flag(Color::black, true);
		_board.set_castle_queenside_flag(Color::black, true);






		// Set white positions

		// Back row
		_board.new_piece(PieceType::rook,	Color::white, (File::a, Rank::r1));
		_board.new_piece(PieceType::knight, Color::white, (File::b, Rank::r1));
		_board.new_piece(PieceType::bishop, Color::white, (File::c, Rank::r1));
		_board.new_piece(PieceType::queen,	Color::white, (File::d, Rank::r1));
		_board.new_piece(PieceType::bishop, Color::white, (File::f, Rank::r1));
		_board.new_piece(PieceType::rook,	Color::white, (File::h, Rank::r1));
		_board.new_piece(PieceType::knight, Color::white, (File::g, Rank::r1));

		// Pawns
		for (auto& _file : files_v)
		{
			_board.new_piece(PieceType::pawn, Color::black, (_file, Rank::r7));
		};
		for (auto& _file : files_v)
		{
			_board.new_piece(PieceType::pawn, Color::white, (_file, Rank::r2));
		};

		// Castle flags
		_board.set_castle_kingside_flag(Color::white, true);
		_board.set_castle_queenside_flag(Color::white, true);


		return _board;
	};



	class IGame
	{
	public:

		const Board& board() const
		{
			return this->board_;
		};

		Board board_;
		
		IGame() = default;
	protected:
		~IGame() = default;
	};

	struct Response
	{
		std::optional<Move> move;

		Response() = default;
	};

	class IChessEngine
	{
	public:

		virtual void set_board(const chess::Board& _board) = 0;
		virtual Response get_move() = 0;
		virtual void start(chess::Board _initialBoard, chess::Color _color) = 0;
		virtual void stop() = 0;

		IChessEngine() = default;
		virtual ~IChessEngine() = default;
	};
};
