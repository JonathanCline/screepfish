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
	inline auto random_iter(auto& _range)
	{
		static std::random_device rnd{};
		static std::mt19937 mt{ rnd() };

		const auto _randomInt = mt();
		const auto _randomPct = (static_cast<double>(_randomInt) + static_cast<double>(mt.min())) /
			static_cast<double>(mt.max());

		const auto _rangeSize = std::ranges::distance(_range);
		const auto _randomIndex = static_cast<size_t>(ceil(static_cast<double>(_rangeSize) * _randomPct) - 1.0);

		if (_rangeSize == 0)
		{
			return std::ranges::end(_range);
		};

		return std::ranges::next(std::ranges::begin(_range), _randomIndex);
	};

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

	

	void ScreepFish::cache(const chess::Board& _board)
	{
	};
	const ScreepFish::BoardCache* ScreepFish::get_cached(size_t _hash)
	{
		return nullptr;
	};


	std::optional<chess::Move> ScreepFish::play_turn(chess::IGame& _game)
	{
		using namespace chess;

		const auto& _board = _game.board();


		const auto& _myColor = this->my_color_;
		if (is_check(_board, _myColor))
		{
			std::cout << "Board is in check!\n";
		};

		const auto _clock = std::chrono::steady_clock{};
		const auto t0 = _clock.now();

		const auto _depth = 4;
		auto _tree = this->build_move_tree(_board, _myColor, _depth);
		
		const auto _move = _tree.best_move();
		
		const auto t1 = _clock.now();
		const auto td = t1 - t0;
		std::cout << "Delta time : " << std::chrono::duration_cast<std::chrono::duration<double>>(td) << '\n';
		std::cout << _tree.tree_size() << '\n';
		std::cout << _board << '\n';

		return _move;
	};
	





	void ScreepFish::set_color(chess::Color _color)
	{
		this->my_color_ = _color;
	};
	void ScreepFish::set_board(chess::Board _board)
	{
		this->board_ = std::move(_board);
	};

};