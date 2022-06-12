#pragma once

/** @file */

#include <new>
#include <concepts>

namespace sch
{
	template <typename T>
	concept cx_block_allocator = requires(T & v, const T & cv, size_t _size, typename T::value_type * _mem)
	{
		typename T::value_type;
		{ v.allocate(_size) } -> std::same_as<typename T::value_type*>;
		{ v.deallocate(_mem) } -> std::same_as<void>;
	};


	template <typename T>
	struct block_allocator
	{
		using value_type = T;
		using pointer = value_type*;
		using size_type = size_t;

		pointer allocate(size_type n) const
		{
			return new value_type[n];
		};
		void deallocate(pointer p)
		{
			delete[] p;
		};
	};

	static_assert(cx_block_allocator<block_allocator<int>>);

};