#pragma once

/** @file */

// SCREEPFISH_UNREACHABLE
#if defined(_MSC_VER) // MSVC
	#define SCREEPFISH_UNREACHABLE __assume(false)
#else // GCC, Clang, ICC
	#define SCREEPFISH_UNREACHABLE __builtin_unreachable()
#endif

#include <cassert>

namespace sch
{
	void report_fatal_check_failure(const char* _file, unsigned long long _line, const char* _cond);
	void report_fatal_assert_failure(const char* _file, unsigned long long _line, const char* _cond);
};

#define SCREEPFISH_DEBUG
#ifdef NDEBUG
#undef SCREEPFISH_DEBUG
#endif


#ifdef SCREEPFISH_DEBUG
	#define SCREEPFISH_ASSERT(cond) { if(!(cond)) { ::sch::report_fatal_assert_failure(__FILE__, __LINE__, #cond); ::abort(); }; }
#else
	#define SCREEPFISH_ASSERT(cond) {}
#endif

#define SCREEPFISH_CHECK(cond) { if(!(cond)) { ::sch::report_fatal_check_failure(__FILE__, __LINE__, #cond); ::abort(); } }



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


#ifdef _MSC_VER
#define SCREEPFISH_BREAK() __debugbreak()
#else
#include <csignal>
#define SCREEPFISH_BREAK() { ::raise(SIGBREAK); }
#endif


namespace sch
{
	/**
	 * @brief Converts an enum to its underlying value representation.
	 * @tparam T Enum type.
	 * @param v Enum value.
	 * @return Enum value as underlying type.
	*/
	template <typename T>
	constexpr auto enumval(T v) -> std::underlying_type_t<T>
	{
		return static_cast<std::underlying_type_t<T>>(v);
	};
};

#define _SCREEPFISH_DEFINE_ENUM_BITFLAG_OR(enumName) constexpr enumName operator|(enumName lhs, enumName rhs) noexcept \
{ return static_cast<enumName>(::sch::enumval(lhs) | ::sch::enumval(rhs)); }

#define _SCREEPFISH_DEFINE_ENUM_BITFLAG_AND(enumName) constexpr enumName operator&(enumName lhs, enumName rhs) noexcept \
{ return static_cast<enumName>(::sch::enumval(lhs) & ::sch::enumval(rhs)); }

#define _SCREEPFISH_DEFINE_ENUM_BITFLAG_XOR(enumName) constexpr enumName operator^(enumName lhs, enumName rhs) noexcept \
{ return static_cast<enumName>(::sch::enumval(lhs) ^ ::sch::enumval(rhs)); }

#define _SCREEPFISH_DEFINE_ENUM_BITFLAG_OR_A(enumName) constexpr enumName& operator|=(enumName& lhs, enumName rhs) noexcept { return lhs = (lhs | rhs); }
#define _SCREEPFISH_DEFINE_ENUM_BITFLAG_AND_A(enumName) constexpr enumName& operator&=(enumName& lhs, enumName rhs) noexcept { return lhs = (lhs & rhs); }
#define _SCREEPFISH_DEFINE_ENUM_BITFLAG_XOR_A(enumName) constexpr enumName& operator^=(enumName& lhs, enumName rhs) noexcept { return lhs = (lhs ^ rhs); }

#define _SCREEPFISH_DEFINE_ENUM_BITFLAG_NOT(enumName) constexpr enumName operator~(enumName rhs) noexcept \
{ return static_cast<enumName>(~::sch::enumval(rhs)); }

#define SCREEPFISH_DEFINE_ENUM_BITFLAG(enumName) \
_SCREEPFISH_DEFINE_ENUM_BITFLAG_OR(enumName) \
_SCREEPFISH_DEFINE_ENUM_BITFLAG_AND(enumName) \
_SCREEPFISH_DEFINE_ENUM_BITFLAG_XOR(enumName) \
_SCREEPFISH_DEFINE_ENUM_BITFLAG_OR_A(enumName) \
_SCREEPFISH_DEFINE_ENUM_BITFLAG_AND_A(enumName) \
_SCREEPFISH_DEFINE_ENUM_BITFLAG_XOR_A(enumName) \
_SCREEPFISH_DEFINE_ENUM_BITFLAG_NOT(enumName)
