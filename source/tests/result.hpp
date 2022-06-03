#pragma once

/** @file */

#include <string>
#include <string_view>

namespace sch
{
	/**
	 * @brief Returned by each test to indicate its outcome.
	*/
	struct TestResult
	{
	public:

		std::string_view name() const { return this->name_; };
		std::string_view description() const { return this->description_; };
		int result() const { return this->result_; };

		explicit operator bool() const noexcept
		{
			return this->result() == 0;
		};

		TestResult(std::string_view _name, int _result, std::string_view _description) :
			name_(_name), result_(_result), description_(_description)
		{};
		TestResult(std::string_view _name, int _result) :
			TestResult(_name, _result, "")
		{};
		TestResult(std::string_view _name) :
			TestResult(_name, 0)
		{};

	private:
		std::string name_;
		std::string description_;
		int result_;
	};

};
