#include "engine.hpp"

#include "chess/fen.hpp"
#include "chess/bitboard.hpp"

#include "utility/string.hpp"

#include <array>
#include <vector>
#include <random>
#include <bitset>
#include <bit>
#include <iostream>
#include <chrono>
#include <fstream>

#include <jclib/functional.h>


namespace sch
{
	chess::MoveTree ScreepFish::build_move_tree(const chess::Board& _board, chess::Color _forPlayer, int _depth)
	{
		using namespace chess;

		auto _tree = chess::MoveTree();
		_tree.board = _board;
		_tree.build_tree((size_t)_depth);

		return _tree;
	};

	void ScreepFish::set_board(const chess::Board& _board)
	{
		const auto lck = std::unique_lock(this->mtx_);
		this->board_ = _board;
	};

	chess::Response ScreepFish::get_move()
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
					return this->best_move_.value();
				};
			};

			// Wait a bit
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		};

		// Timed out
		return chess::Response{};
	};

	void ScreepFish::start(chess::Board _initialBoard, chess::Color _color)
	{
		this->board_ = _initialBoard;
		this->my_color_ = _color;

		std::cout << "About to start screepfish thread" << std::endl;
		this->thread_ = std::jthread([this](std::stop_token _stop)
			{
				std::cout << "Started screepfish thread" << std::endl;
				this->thread_main(_stop);
			});

		this->init_barrier_.arrive_and_wait();
	};
	void ScreepFish::stop()
	{
		this->thread_.request_stop();
		if (this->thread_.joinable())
		{
			this->thread_.join();
		};
	};
	
	void ScreepFish::thread_main(std::stop_token _stop)
	{
		this->init_barrier_.arrive_and_wait();

		// Storage for tracking calcultion times
		auto _times = std::vector<std::chrono::steady_clock::duration>();

		while (!_stop.stop_requested())
		{
			{
				const auto lck = std::unique_lock(this->mtx_);
				if (!this->best_move_)
				{
					using namespace chess;

					const auto& _board = this->board_;
					const auto& _myColor = this->my_color_;
					
					const size_t _pieceCount = _board.pieces().size();
					auto _depth = 5;

					// Bump depth as many moves will be discarded
					const bool _isCheck = is_check(_board, _myColor);

					//if (_isCheck)
					//{
					//	_depth += 1;
					//};
					
					// Bump depth if no queens on board
					//if (_board.pfind(Piece::queen, Color::white) == _board.pend() && _board.pfind(Piece::queen, Color::black) == _board.pend()
					//	&& _pieceCount <= 15)
					//{
					//	_depth += 1;
					//};
					if (_pieceCount <= 8 && !_isCheck)
					{
						_depth += 1;
					}
					if (_pieceCount <= 4 && !_isCheck)
					{
						_depth += 1;
					};

					const auto _clock = std::chrono::steady_clock{};
					

					const auto t0 = _clock.now();
					auto _tree = this->build_move_tree(_board, _myColor, _depth);
					std::cout << "Tree hash set size = " << _tree.hash_set.size() << '\n';


					const auto t1 = _clock.now();
					const auto _move = _tree.best_move(this->rnd_);
					const auto t2 = _clock.now();

					const auto tdA = t1 - t0;
					const auto tdB = t2 - t1;
					const auto td = t2 - t0;

					_times.push_back(td);

					constexpr auto fn = [](auto v)
					{
						return std::chrono::duration_cast<std::chrono::duration<double>>(v);
					};
					std::cout << "Delta time : " << fn(td) << '(' << fn(tdA) << ", " << fn(tdB) << ")\n";
					
					// Log if set
					if (auto& _loggingDir = this->logging_dir_; _loggingDir)
					{
						// Log the text data
						{
							namespace fs = std::filesystem;
							const auto _dirPath = *_loggingDir / ("m" + std::to_string(_board.get_full_move_count()));
							if (fs::exists(_dirPath))
							{
								fs::remove_all(_dirPath);
							};
							fs::create_directories(_dirPath);

							// Initial position
							{
								const auto _path = _dirPath / "initial.txt";
								auto _file = std::ofstream(_path);
								_file << _board << '\n' << '\n';
								_file << get_fen(_board) << '\n';
							};

							// Lines
							const auto _topLines = _tree.get_top_lines(3);
							size_t _lineN = 0;
							for (auto& _line : _topLines)
							{
								const auto _path = _dirPath / ("line" + std::to_string(_lineN++) + ".txt");
								auto _file = std::ofstream(_path);
								
								_file << *_move << " = " << _move->rating() << '\n';

								auto b = _board;
								for (auto& v : _line)
								{
									b.move(v);
									_file << v << '\n';
									_file << v.rating() << '\n';
								};
								_file << '\n' << '\n';
								
								b = _board;
								for (auto& v : _line)
								{
									b.move(v);
									_file << v << '\n';
									_file << v.rating() << '\n' << '\n';
									_file << b << '\n' << '\n';
									_file << get_fen(b) << '\n' << '\n' << str::rep('=', 80) << '\n' << '\n';
								};
							};
						};
					};

					Response _resp{};
					_resp.move = _move;
					this->best_move_ = _resp;
				};
			};

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		};

		using dur = std::chrono::duration<double>;
		dur _avgDur{};
		for (auto& v : _times)
		{
			_avgDur += std::chrono::duration_cast<dur>(v);
		};
		_avgDur /= _times.size();

		std::cout << "Average calculation time = " << _avgDur << '\n';
	};

	void ScreepFish::set_logging_dir(std::filesystem::path _path)
	{
		namespace fs = std::filesystem;
		if (!fs::exists(_path))
		{
			fs::create_directories(_path);
		};

		const auto lck = std::unique_lock(this->mtx_);
		this->logging_dir_ = _path;
	};

	ScreepFish::ScreepFish() :
		init_barrier_(2),
		rnd_(std::random_device{}()),
		logging_dir_{}
	{
		
	};
	ScreepFish::~ScreepFish()
	{
		this->stop();
	};

};