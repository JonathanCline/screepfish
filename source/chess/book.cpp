#include "book.hpp"

namespace chess
{
	Book::BranchBlockAllocator::value_type* Book::BranchBlockAllocator::allocate(size_t n) const
	{
		return new Branch[n]{};
	};
	void Book::BranchBlockAllocator::deallocate(value_type* p) const
	{
		delete[] p;
	};
};

namespace chess
{

};