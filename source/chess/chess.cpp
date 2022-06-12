#include "chess.hpp"

namespace chess
{
	Board& reset_board(Board& _board)
	{
		// Clear the board so we can begin anew.
		_board.clear();

		// Kings in front as they are commonly needed
		_board.new_piece(PieceType::king, Color::white, (File::e, Rank::r1));
		_board.new_piece(PieceType::king, Color::black, (File::e, Rank::r8));



		// Set black positions

		// Back row
		_board.new_piece(PieceType::rook, Color::black, (File::a, Rank::r8));
		_board.new_piece(PieceType::knight, Color::black, (File::b, Rank::r8));
		_board.new_piece(PieceType::bishop, Color::black, (File::c, Rank::r8));
		_board.new_piece(PieceType::queen, Color::black, (File::d, Rank::r8));
		_board.new_piece(PieceType::bishop, Color::black, (File::f, Rank::r8));
		_board.new_piece(PieceType::rook, Color::black, (File::h, Rank::r8));
		_board.new_piece(PieceType::knight, Color::black, (File::g, Rank::r8));

		// Castle flags
		_board.set_castle_kingside_flag(Color::black, true);
		_board.set_castle_queenside_flag(Color::black, true);






		// Set white positions

		// Back row
		_board.new_piece(PieceType::rook, Color::white, (File::a, Rank::r1));
		_board.new_piece(PieceType::knight, Color::white, (File::b, Rank::r1));
		_board.new_piece(PieceType::bishop, Color::white, (File::c, Rank::r1));
		_board.new_piece(PieceType::queen, Color::white, (File::d, Rank::r1));
		_board.new_piece(PieceType::bishop, Color::white, (File::f, Rank::r1));
		_board.new_piece(PieceType::rook, Color::white, (File::h, Rank::r1));
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



		// Sync
		_board.sync();

		return _board;
	};
}