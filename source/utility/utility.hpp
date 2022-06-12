#pragma once

/** @file */

// SCREEPFISH_UNREACHABLE
#if defined(_MSC_VER) // MSVC
	#define SCREEPFISH_UNREACHABLE __assume(false)
#else // GCC, Clang, ICC
	#define SCREEPFISH_UNREACHABLE __builtin_unreachable()
#endif

#include <cassert>

#define SCREEPFISH_ASSERT(cond) assert(cond)


#include <array>
#include <ranges>
#include <algorithm>

namespace sch
{
	template <typename T, size_t N, size_t N2>
	constexpr auto concat_arrays(std::array<T, N> lhs, std::array<T, N2> rhs)
	{
		std::array<T, N + N2> o;
		auto it = o.begin();
		std::ranges::move(lhs, it);
		std::ranges::move(rhs, it + N);
		return o;
	};

	template <typename T, size_t N>
	constexpr auto prepend_array(std::array<T, N> _arr, T _val)
	{
		return concat_arrays(std::array<T, 1>{ _val }, _arr);
	};

};
