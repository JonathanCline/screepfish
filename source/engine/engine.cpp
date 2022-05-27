#include "engine.hpp"

#include "chess/bitboard.hpp"

#include <array>
#include <vector>
#include <random>
#include <bitset>
#include <bit>
#include <iostream>
#include <chrono>

#include <jclib/functional.h>


namespace sch
{
	chess::MoveTree ScreepFish::build_move_tree(const chess::Board& _board, chess::Color _forPlayer, int _depth)
	{
		using namespace chess;

		auto _tree = chess::MoveTree();
		_tree.board = _board;
		_tree.to_play = _forPlayer;

		for (int n = 0; n != _depth; ++n)
		{
			_tree.evalulate_next();
		};

		return _tree;
	};

	void ScreepFish::set_board(const chess::Board& _board)
	{
		const auto lck = std::unique_lock(this->mtx_);
		this->board_ = _board;
	};

	std::optional<chess::Move> ScreepFish::get_move()
	{
		// Clear the previous best move
		{
			const auto lck = std::unique_lock(this->mtx_);
			this->best_move_.reset();
		};

		for(int n = 0; n != 1000; ++n)
		{
			// Try to get the next move
			{
				const auto lck = std::unique_lock(this->mtx_);
				if (this->best_move_)
				{
					return this->best_move_;
				};
			};

			// Wait a bit
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		};

		// Timed out
		return std::nullopt;
	};

	void ScreepFish::start(chess::Board _initialBoard, chess::Color _color)
	{
		this->board_ = _initialBoard;
		this->my_color_ = _color;

		this->thread_ = std::jthread([this](std::stop_token _stop)
			{
				this->thread_main(_stop);
			});


		this->init_barrier_.arrive_and_wait();
	};
	void ScreepFish::stop()
	{
		this->thread_.request_stop();
	};
	
	void ScreepFish::thread_main(std::stop_token _stop)
	{
		this->init_barrier_.arrive_and_wait();
		while (!_stop.stop_requested())
		{
			{
				const auto lck = std::unique_lock(this->mtx_);
				if (!this->best_move_)
				{
					using namespace chess;

					const auto& _board = this->board_;
					const auto& _myColor = this->my_color_;
					
					if (is_check(_board, _myColor))
					{
						std::cout << "Board is in check!\n";
					};

					const auto _clock = std::chrono::steady_clock{};
					const auto t0 = _clock.now();

					const size_t _pieceCount = _board.pieces().size();

					const auto _depth = (_pieceCount < 8) ? 5 : 4;
					auto _tree = this->build_move_tree(_board, _myColor, _depth);
					std::cout << _tree.tree_size() << '\n';

					const auto _move = _tree.best_move();

					const auto t1 = _clock.now();
					const auto td = t1 - t0;
					std::cout << "Delta time : " << std::chrono::duration_cast<std::chrono::duration<double>>(td) << '\n';
					std::cout << _board << '\n';
					std::cout << _board.get_white_piece_bitboard() << '\n';

					this->best_move_ = _move;
				};
			};

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		};
	};


	ScreepFish::ScreepFish() :
		init_barrier_(2)
	{
		
	};

};