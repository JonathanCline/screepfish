#include "engine.hpp"

#include "chess/fen.hpp"
#include "chess/bitboard.hpp"

#include "utility/string.hpp"
#include "utility/logging.hpp"

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

		// Configure tree profile.
		auto _profile = MoveTreeProfile();
		_profile.follow_captures_ = false;
		_profile.follow_checks_ = true;
		_profile.enable_pruning_ = false;
		_profile.alphabeta_ = true;

		// BUILD THE TREE
		auto _tree = chess::MoveTree(_board);
		_tree.build_tree((size_t)_depth, _depth + 1, _profile);

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

		sch::log_info("About to start screepfish thread");
		this->thread_ = std::jthread([this](std::stop_token _stop)
			{
				sch::log_info("Started screepfish thread");
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
	



	void ScreepFish::calculate_next_move()
	{
		using namespace chess;

		const auto& _board = this->board_;
		const auto& _myColor = this->my_color_;

		const size_t _pieceCount = _board.pieces().size();
		auto _depth = this->search_depth_;

		// Bump depth as many moves will be discarded
		const bool _isCheck = is_check(_board, _myColor);

		if (_pieceCount <= 8 && !_isCheck)
		{
			_depth += 1;
		}
		if (_pieceCount <= 4 && !_isCheck)
		{
			_depth += 1;
		};


		// TODO : Split this function into at least two parts - one for book moves, one for evaluated moves

		auto _move = std::optional<RatedMove>(std::nullopt);
		auto _tree = chess::MoveTree();

		bool _isBookMove = false;

		const auto _clock = std::chrono::steady_clock{};

		auto t0 = _clock.now();
		auto t1 = _clock.now();
		auto t2 = _clock.now();

		// Get next move from book if possible.
		if (const auto _bookMove = this->pop_next_book_move(_board); _bookMove)
		{
			t0 = _clock.now();
			auto _newBoard = _board;
			_newBoard.move(_bookMove);

			const auto _rating = chess::quick_rate(_newBoard, _myColor);
			_move = RatedMove(_bookMove, _rating);

			t1 = _clock.now();
			t2 = _clock.now();

			SCREEPFISH_BREAK();
			_isBookMove = true;
		};
		// Evaluate next move if we didn't have a book move ready.
		if (!_isBookMove)
		{
			t0 = _clock.now();
			_tree = this->build_move_tree(_board, _myColor, _depth);
			t1 = _clock.now();
			_move = _tree.best_move(this->rnd_);
			t2 = _clock.now();
		};

		const auto tdA = t1 - t0;
		const auto tdB = t2 - t1;
		const auto td = t2 - t0;

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

				// Perf
				{
					const auto _path = _dirPath / "perf.txt";
					auto _file = std::ofstream(_path);
					_file << "Total		  : " << td << '\n';
					if (!_isBookMove)
					{
						_file << "Tree Build  : " << tdA << '\n';
						_file << "Tree Search : " << tdB << '\n';
					};
				};

				// Initial position
				{
					const auto _path = _dirPath / "initial.txt";
					auto _file = std::ofstream(_path);
					if (_isBookMove)
					{
						_file << "Book Move\n\n";
					}
					else
					{
						_file << "Depth : " << _depth << "\n\n";
					};
					_file << _board << '\n' << '\n';
					_file << get_fen(_board) << '\n';
				};

				// Top level moves
				if(!_isBookMove)
				{
					const auto _path = _dirPath / "moves.txt";
					auto _file = std::ofstream(_path);

					_file << "Total Tree Size : " << _tree.tree_size() << '\n';
					for (auto& _move : _tree.root())
					{
						_file << _move.move_ << " : " << _move.rating() << " : " << _move.quick_rating() << '\n';
					};
				};

				// Second level moves
				if (!_isBookMove)
				{
					const auto _path = _dirPath / "moves2.txt";
					auto _file = std::ofstream(_path);

					for (auto& _fmove : _tree.root())
					{
						if (_fmove.empty())
						{
							_file << "-\n";
						}
						else
						{
							_file << _fmove.move_ << ":\n";
							for (auto& _move : _fmove)
							{
								_file << '\t' << _move.move_ << " : " << _move.rating() << " : " << _move.quick_rating() << '\n';
							};
							_file << '\n';
						};
					};
				};

				// Lines
				if (!_isBookMove)
				{
					const auto _topLines = _tree.get_top_lines(3);
					size_t _lineN = 0;
					for (auto& _line : _topLines)
					{
						const auto _path = _dirPath / ("line" + std::to_string(_lineN++) + ".txt");
						auto _file = std::ofstream(_path);

						_file << "Final Rating : " << _move->rating() << '\n';

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
		};

		Response _resp{};
		_resp.move = _move;
		this->best_move_ = _resp;
	};


	void ScreepFish::thread_main(std::stop_token _stop)
	{
		this->init_barrier_.arrive_and_wait();

		// Storage for tracking calcultion times
		auto _times = std::vector<double>();

		while (!_stop.stop_requested())
		{
			{
				const auto lck = std::unique_lock(this->mtx_);
				if (!this->best_move_)
				{
					this->calculate_next_move();
				};
			};

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		};

		if (!_times.empty())
		{
			const auto _durAcc = jc::accumulate(_times);
			using dur = std::chrono::duration<double>;
			const auto _avgDur = _durAcc / static_cast<dur::rep>(_times.size());
			sch::log_info(str::concat_to_string("Average calculation time = ", dur(_avgDur)));
		};
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

	void ScreepFish::set_search_depth(size_t _depth)
	{
		this->search_depth_ = _depth;
	};

	ScreepFish::ScreepFish() :
		init_barrier_(2),
		rnd_(std::random_device{}()),
		logging_dir_{},
		search_depth_{ 6 }
	{
		
	};
	ScreepFish::~ScreepFish()
	{
		this->stop();
	};

};