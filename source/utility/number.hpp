#pragma once

/** @file */


#include <algorithm>

namespace chess
{
	struct inclusive_t { constexpr explicit inclusive_t() noexcept = default; };
	constexpr inline inclusive_t inclusive{};

	struct exclusive_t { constexpr explicit exclusive_t() noexcept = default; };
	constexpr inline exclusive_t exclusive{};


	/**
	 * @brief Checks if a value is within the bounds specified by a lower and upper value.
	 * 
	 * @tparam T Type to check.
	 * @param _value Value to check is in bounds.
	 * @param _min Bounds minimum value, inclusive.
	 * @param _max Bounds maximum value, exclusive.
	 * @return True if in bounds, false otherwise.
	*/
	template <typename T>
	constexpr bool inbounds(const T& _value, const T& _min, const T& _max)
	{
		return (_value >= _min) && (_value < _max);
	};

	/**
	 * @brief Checks if a value is within the bounds specified by a lower and upper value.
	 *
	 * Note: This is the inclusive version, _min and _max are both inclusive values.
	 * 
	 * @tparam T Type to check.
	 * @param _value Value to check is in bounds.
	 * @param _min Bounds minimum value, inclusive.
	 * @param _max Bounds maximum value, inclusive.
	 * @param _mode Inclusive tag type value.
	 * @return True if in bounds, false otherwise.
	*/
	template <typename T>
	constexpr bool inbounds(const T& _value, const T& _min, const T& _max, inclusive_t)
	{
		return (_value >= _min) && (_value <= _max);
	};

};
