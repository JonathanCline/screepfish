#pragma once

/** @file */

#include "result.hpp"

#include <memory>

namespace sch
{
	class ITest
	{
	public:

		virtual TestResult run() = 0;

		ITest() = default;
		virtual ~ITest() = default;
	};


}