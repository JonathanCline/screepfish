#include "utility.hpp"

#include <iostream>

namespace sch
{
	void report_fatal_check_failure(const char* _file, unsigned long long _line, const char* _cond)
	{
		std::cout << "[Fatal] SCREEPFISH_CHECK() failed!\n"
			<< '\t' << _cond
			<< '\t' << "file : " << _file << '\n'
			<< '\t' << "line : " << _line << '\n';
	};
	void report_fatal_assert_failure(const char* _file, unsigned long long _line, const char* _cond)
	{
		std::cout << "[Fatal] SCREEPFISH_ASSERT() failed!\n"
			<< '\t' << _cond
			<< '\t' << "file : " << _file << '\n'
			<< '\t' << "line : " << _line << '\n';
	};
};