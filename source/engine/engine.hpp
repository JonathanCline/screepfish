#pragma once

/** @file */

#include "chess/book.hpp"
#include "chess/chess.hpp"
#include "chess/move_tree.hpp"

#include <mutex>
#include <thread>
#include <barrier>
#include <variant> 
#include <atomic>
#include <random>
#include <filesystem>


namespace sch
{
	class ScreepFish : public chess::IChessEngine
	{
	private:

		chess::MoveTree build_move_tree(const chess::Board& _board, chess::Color _forPlayer, int _depth);
		
		void thread_main(std::stop_token _stop);

		/**
		 * @brief Checks if there is an opening book for the engine to follow.
		 * @return True if book, false otherwise.
		*/
		bool has_opening_book() const
		{
			return this->opening_book_.has_value();
		};

		/**
		 * @brief Gets the next book move.
		 * 
		 * This is a helper as we don't have deducing this.
		 * 
		 * @param _board Board position to look from.
		 * @param _bookOpt Optional<Book>
		 * @return Next book move, or null if no response is found.
		*/
		template <typename BookOptT>
		static auto* next_book_move_branch_impl(const chess::Board& _board, BookOptT& _bookOpt)
		{
			using return_type = std::conditional_t<std::is_const_v<BookOptT>,
				const chess::Book::Branch*, chess::Book::Branch*>;

			// Return null move if no book is set.
			if (!_bookOpt)
			{
				return return_type{};
			};

			// No opponent move = pick any next move
			const auto _opponentMove = _board.get_last_move();
			if (!_opponentMove)
			{
				return return_type{};
			};

			auto& _book = *_bookOpt;

			// Find the response from the book.
			const auto it = _book.root().find_response(_opponentMove);
			if (it == _book.root().end())
			{
				// No response preferred
				return return_type{};
			}
			else 
			{
				return &it->second;
			};
		};

		using BookBranch = chess::Book::Branch;

		/**
		 * @brief Gets the next book move.
		 * @param _board Board position to look from.
		 * @return Next book move, or null if no response is found.
		*/
		BookBranch* next_book_move_branch(const chess::Board& _board)
		{
			return this->next_book_move_branch_impl(_board, this->opening_book_);
		};

		/**
		 * @brief Gets the next book move.
		 * @param _board Board position to look from.
		 * @return Next book move, or null if no response is found.
		*/
		const BookBranch* next_book_move_branch(const chess::Board& _board) const
		{
			return this->next_book_move_branch_impl(_board, this->opening_book_);
		};

		/**
		 * @brief Gets the next book move and rebases the book onto the returned move.
		 * 
		 * If no move was returned then the opening book is depleted and will be set to null.
		 * If a move was returned, the opening book's root node is now set to that move.
		 * 
		 * @param _board Board position to look from.
		 * @return Next book move, or null move if no move was found.
		*/
		chess::Move pop_next_book_move(const chess::Board& _board)
		{
			auto& _book = this->opening_book_;
			if (!_book) { return jc::null; };

			auto _branch = this->next_book_move_branch(_board);
			if (_branch && _branch->move())
			{
				const auto _move = _branch->move();

				// Rebase book onto returned response.
				_book->root() = *_branch;
				return _move;
			}
			else
			{
				// Null out book as it is no longer useful
				_book.reset();
				return jc::null;
			};
		};

		/**
		 * @brief Calculates the next move, should be called when "best_move_" is null (empty).
		*/
		void calculate_next_move();

	public:

		void set_board(const chess::Board& _board) final;
		chess::Response get_move() final;

		void start(chess::Board _initialBoard, chess::Color _color) final;
		void stop() final;

		void set_logging_dir(std::filesystem::path _path);


		void set_search_depth(size_t _depth);

		/**
		 * @brief Sets the opening book for the engine to use.
		 * @param _book Opening book.
		*/
		void set_opening_book(const chess::Book& _book)
		{
			this->opening_book_ = _book;
		};




		ScreepFish();
		~ScreepFish();

	private:

		chess::Board board_;
		chess::Color my_color_;

		std::barrier<> init_barrier_;
		mutable std::mutex mtx_;

		std::optional<chess::Response> best_move_;
		std::optional<std::filesystem::path> logging_dir_{};

		std::jthread thread_;

		std::mt19937 rnd_;


		/**
		 * @brief The opening book to follow.
		*/
		std::optional<chess::Book> opening_book_{};


		// Configuration settings
		size_t search_depth_ = 5;
	};

};