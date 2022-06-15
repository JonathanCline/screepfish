#pragma once

/** @file */

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

	public:

		void set_board(const chess::Board& _board) final;
		chess::Response get_move() final;

		void start(chess::Board _initialBoard, chess::Color _color) final;
		void stop() final;

		void set_logging_dir(std::filesystem::path _path);


		void set_search_depth(size_t _depth);



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

		size_t search_depth_ = 5;
	};

};