#include "test_book.hpp"

#include "chess/book.hpp"

namespace sch
{
	TestResult tests::test_opening_book()
	{
		const auto _name = "test_opening_book";
		
		using namespace chess;

		{
			auto _book = Book();
			if (!_book.root().empty())
			{
				return TestResult(_name, 1, "null book root node must be empty");
			};
			_book.clear();
			if (!_book.root().empty())
			{
				return TestResult(_name, 1, "cleared book root node must be empty");
			};
		};

		{
			auto _book = Book();
			using entry = std::pair<Move, Move>;

			{
				auto _board = Board();
				reset_board(_board);

				auto _bufferData = std::array<Move, 64>{};
				auto _buffer = MoveBuffer(_bufferData);
				get_moves(_board, _board.get_toplay(), _buffer);

				auto _pairs = std::array<entry, 64>{};
				std::transform(_bufferData.data(), _buffer.head(), _pairs.begin(),
					[](auto& v)
					{
						return entry
						{
							v,
							Move((File::e, Rank::r7), (File::e, Rank::r5))
						};
					});

				_book.root().assign(_pairs.begin(),
					_pairs.begin() + (_buffer.head() - _bufferData.data()));

				if (_book.root().empty())
				{
					return TestResult(_name, 1, "book with assigned moves should not be empty"); 
				};
				if (!_book.root().has_response(Move((File::e, Rank::r2), (File::e, Rank::r4))))
				{
					return TestResult(_name, 1, "expected book to have a response to e2e4 on move 1");
				};

				_book.clear();
				if (!_book.root().empty())
				{
					return TestResult(_name, 1, "cleared book root node must be empty");
				};
			};

		};

		// Assume pass
		return TestResult(_name);
	};
};
