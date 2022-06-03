#pragma once

/** @file */

#include "result.hpp"

#include <vector>

namespace sch
{

	/**
	 * @brief Runs all tests.
	 * @param _stopOnFail Whether or not to stop on test failure, defaults to false.
	 * @return Test results.
	*/
	std::vector<TestResult> run_tests(bool _stopOnFail = false);

};