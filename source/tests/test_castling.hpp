#pragma once

/** @file */

#pragma once

/** @file */

#include "test_base.hpp"

#include "chess/fen.hpp"
#include "chess/chess.hpp"
#include "chess/move.hpp"

#include <vector>
#include <string_view>


namespace sch
{
	class Test_Castling : public ITest
	{
	public:

		TestResult run() final
		{
			using namespace chess;

			const auto cwk = chess::can_castle_kingside(this->board_, Color::white);
			const auto cwq = chess::can_castle_queenside(this->board_, Color::white);
			const auto cbk = chess::can_castle_kingside(this->board_, Color::black);
			const auto cbq = chess::can_castle_queenside(this->board_, Color::black);
			
			auto s =
				std::string(" castle mismatch ") +
				"\n fen = " + chess::get_fen(this->board_);

			if (this->wk_ != cwk)
			{
				s = "White kingside" + s;
				return TestResult(this->name_, -1, s);
			};
			if (this->wq_ != cwq)
			{
				s = "White queenside" + s;
				return TestResult(this->name_, -1, s);
			};
			if (this->bk_ != cbk)
			{
				s = "Black kingside" + s;
				return TestResult(this->name_, -1, s);
			};
			if (this->bq_ != cbq)
			{
				s = "Black queenside" + s;
				return TestResult(this->name_, -1, s);
			};

			return TestResult(this->name_);
		};

		Test_Castling(std::string_view _name, chess::Board _board, bool _whiteKing, bool _whiteQueen, bool _blackKing, bool _blackQueen) :
			name_(_name), board_(_board),
			wk_(_whiteKing), wq_(_whiteQueen),
			bk_(_blackKing), bq_(_blackQueen)
		{};

	private:
		bool wk_, wq_, bk_, bq_;
		std::string name_;
		chess::Board board_;
	};
};