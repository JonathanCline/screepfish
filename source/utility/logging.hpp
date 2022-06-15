#pragma once

/** @file */

#include <string_view>

namespace sch
{
	void log_info(std::string_view _what);
	void log_warning(std::string_view _what);
	void log_error(std::string_view _what);

	void log_output_chunk_divider();
	void log_output_chunk(std::string_view _what);

};