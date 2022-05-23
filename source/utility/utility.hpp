#pragma once

/** @file */

// SCREEPFISH_UNREACHABLE
#if defined(_MSC_VER) // MSVC
	#define SCREEPFISH_UNREACHABLE __assume(false)
#else // GCC, Clang, ICC
	#define SCREEPFISH_UNREACHABLE __builtin_unreachable()
#endif
