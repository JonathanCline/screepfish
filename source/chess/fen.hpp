#pragma once

/** @file */

#include "chess.hpp"

#include <string>
#include <optional>
#include <string_view>

namespace chess
{
	constexpr auto standard_start_pos_fen_v = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	std::optional<Board> parse_fen(const std::string_view _fen);
	std::string get_fen(const Board& _board);
};