#pragma once

/** @file */

#include <string>
#include <string_view>

namespace str
{

	constexpr std::string_view lstrip(std::string_view _str)
	{
		size_t n = 0;
		for (; n != _str.size(); ++n)
		{
			if (!isspace(_str[n]))
			{
				break;
			};
		};
		_str.remove_prefix(n);
		return _str;
	};

	constexpr std::string_view rstrip(std::string_view _str)
	{
		size_t n = 0;
		for (; n != _str.size(); ++n)
		{
			if (!isspace(_str[_str.size() - (n + 1)]))
			{
				break;
			};
		};
		_str.remove_suffix(n);
		return _str;
	};

	constexpr std::string_view strip(std::string_view _str)
	{
		return rstrip(lstrip(_str));
	};

	inline std::string rep(std::string _str, size_t _times)
	{
		auto s = std::string{};
		for (size_t n = 0; n != _times; ++n)
		{
			s.append(_str);
		};
		return s;
	};	
	inline std::string rep(char c, size_t _times)
	{
		auto s = std::string(_times, c);
		return s;
	};
};