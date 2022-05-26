#pragma once

/** @file */

#include "chess.hpp"

#include <string>
#include <optional>
#include <string_view>

namespace chess
{
	std::optional<Board> parse_fen(const std::string_view _fen);
	std::string get_fen(const Board& _board);
};