#include "board.hpp"
#include "chess.hpp"

#include <jclib/algorithm.h>

#include <ostream>

namespace chess
{
	std::ostream& operator<<(std::ostream& _ostr, const BoardBase& _value)
	{
		std::array<std::array<char, 8>, 8> _grid{};
		for (auto& vx : _grid) { std::ranges::fill(vx, ' '); };

		for (auto& _piece : _value.pieces())
		{
			const auto f = _piece.file();
			const auto r = _piece.rank();

			auto x = jc::to_underlying(f);
			auto y = jc::to_underlying(r);
			auto c = ' ';

			switch (_piece.type())
			{
			case PieceType::pawn: c = 'P'; break;
			case PieceType::knight: c = 'N'; break;
			case PieceType::bishop: c = 'B'; break;
			case PieceType::rook: c = 'R'; break;
			case PieceType::queen: c = 'Q'; break;
			case PieceType::king: c = 'K'; break;
			default:
				continue;
				break;
			};

			if (_piece.color() == Color::black)
			{
				c = static_cast<char>(std::tolower(c));
			};

			_grid[static_cast<size_t>(7 - y)][x] = c;
		};

		_ostr << "+-+-+-+-+-+-+-+-+\n";
		for (auto& vx : _grid)
		{
			for (auto& c : vx)
			{
				_ostr.put('|');
				_ostr.put(c);
			};
			_ostr.put('|');
			_ostr.put('\n');
			_ostr << "+-+-+-+-+-+-+-+-+\n";
		};

		return _ostr;
	};
};

namespace chess
{
	void BoardBase::move(const Move& _move)
	{
		// Exit early on null move
		if (!_move) JCLIB_UNLIKELY
		{
			return;
		};

		// Aliasing parts to keep compatability
		const auto _fromPos = _move.from();
		const auto _toPos = _move.to();
		const auto _promotion = _move.promotion();

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
						SCREEPFISH_ASSERT(this->get_castle_queenside_flag(_from.color()));
						this->just_move_piece((File::a, Rank::r1), (File::d, Rank::r1));
					}
					else if (_toPos == (File::g, Rank::r1))
					{
						SCREEPFISH_ASSERT(this->get_castle_kingside_flag(_from.color()));
						this->just_move_piece((File::h, Rank::r1), (File::f, Rank::r1));
					};
				}
				else if (_from.color() == Color::black && _fromPos == (File::e, Rank::r8))
				{
					if (_toPos == (File::c, Rank::r8))
					{
						SCREEPFISH_ASSERT(this->get_castle_queenside_flag(_from.color()));
						this->just_move_piece((File::a, Rank::r8), (File::d, Rank::r8));
					}
					else if (_toPos == (File::g, Rank::r8))
					{
						SCREEPFISH_ASSERT(this->get_castle_kingside_flag(_from.color()));
						this->just_move_piece((File::h, Rank::r8), (File::f, Rank::r8));
					};
				};
			}
			// Enpassant handling
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
					this->reset_castle_flag(CastleBit::wking);
					this->reset_castle_flag(CastleBit::wqueen);
				}
				else
				{
					this->reset_castle_flag(CastleBit::bking);
					this->reset_castle_flag(CastleBit::bqueen);
				};
			}
			else if (_from == PieceType::rook)
			{
				if (_from.color() == Color::white)
				{
					if (_fromPos == (File::h, Rank::r1))
					{
						this->reset_castle_flag(CastleBit::wking);
					}
					else if (_fromPos == (File::a, Rank::r1))
					{
						this->reset_castle_flag(CastleBit::wqueen);
					};
				}
				else
				{
					if (_fromPos == (File::h, Rank::r8))
					{
						this->reset_castle_flag(CastleBit::bking);
					}
					else if (_fromPos == (File::a, Rank::r8))
					{
						this->reset_castle_flag(CastleBit::bqueen);
					};
				};
			}
			else if (_to == PieceType::rook)
			{
				if (_to.color() == Color::white)
				{
					if (_toPos == (File::h, Rank::r1))
					{
						this->reset_castle_flag(CastleBit::wking);
					}
					else if (_toPos == (File::a, Rank::r1))
					{
						this->reset_castle_flag(CastleBit::wqueen);
					};
				}
				else
				{
					if (_toPos == (File::h, Rank::r8))
					{
						this->reset_castle_flag(CastleBit::bking);
					}
					else if (_toPos == (File::a, Rank::r8))
					{
						this->reset_castle_flag(CastleBit::bqueen);
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
		SCREEPFISH_ASSERT(it != this->end());

		if (_promotion != PieceType::none && *it == PieceType::pawn)
		{
			this->pieces_by_pos_.at(toindex(_fromPos)) = _promotion;
			*this->pfind(_fromPos) = _promotion;
		};

		if (_oldEnpassantTarget && *_oldEnpassantTarget == _toPos &&
			it->type() == Piece::pawn)
		{
			const auto _enemyPos = (_toPos == Rank::r6) ?
				(_toPos.file(), Rank::r5) :
				(_toPos.file(), Rank::r4);
			if (this->pfind(_enemyPos)->color() != it->color())
			{
				this->erase(_enemyPos);
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

		// Set the played move as the last move
		this->set_last_move(_move);
	};

	void BoardBase::set_previous_board_hash(const Board& _board)
	{
		//auto& _storage = this->last_board_hashes_;
		//std::shift_right(_storage.begin(), _storage.end(), 1);
		//_storage.front() = hash(_board);
	};

	bool BoardBase::is_repeated_move(Move _move) const
	{
		auto& _storage = this->last_moves_;
		return jc::contains(_storage, _move);
	};
	bool BoardBase::is_last_move_repeated_move() const
	{
		auto& _storage = this->last_moves_;

		// If last move is null we can return false early
		if (!_storage.front()) { return false; };

		// Just check if the player's last move is the same
		return _storage.at(2) == _storage.front();
	};

}

