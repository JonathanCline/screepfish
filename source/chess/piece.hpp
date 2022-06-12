#pragma once

/** @file */

#include <array>

namespace chess
{
	/**
	 * @brief Named piece / square colors.
	*/
	enum class Color : bool
	{
		black,
		white
	};

	/**
	 * @brief Array containing the different possible color values.
	*/
	constexpr inline auto colors_v = std::array
	{
		Color::black,
		Color::white
	};

	/**
	 * @brief Inverts the given color.
	 *
	 * white -> black
	 * black -> white
	 *
	 * @param rhs Color to invert.
	 * @return Inverted (opposite) color.
	*/
	constexpr Color operator!(Color rhs) noexcept
	{
		return Color(!static_cast<bool>(rhs));
	};

};
