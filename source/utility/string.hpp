#pragma once

/** @file */

#include <span>
#include <ranges>
#include <sstream>
#include <algorithm>

#include <vector>
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


	constexpr std::string_view longest_substr(const std::string_view _str, const std::string_view _substr)
	{
		std::string_view _longestMatch{};
		if (_substr.empty()) { return 0; };

		{
			auto sIt = _substr.begin();
			size_t _sOffset = 0;
			size_t _currentMatchLength = 0;
			
			for (auto hIt = _str.begin(); hIt != _str.end(); ++hIt)
			{
				if (*hIt == *sIt)
				{
					// start or continuation of match
					++_currentMatchLength;
					if (sIt == _substr.end())
					{
						// longest possible match found
						_longestMatch = _str.substr(_sOffset, _currentMatchLength);
						break;
					};
					++sIt; // next substr char
				}
				else
				{
					if (_currentMatchLength != 0)
					{
						// end of match, set as longest if longer than current best.
						if (_currentMatchLength > _longestMatch.size())
						{
							// new longest match found
							auto _matchedSubstr = _str.substr(_sOffset, _currentMatchLength);
							_longestMatch = _matchedSubstr;
						};

						// reset tracking vars
						sIt = _substr.begin();
						_currentMatchLength = 0;
					}
					else
					{
						// still no match
					};

					_sOffset = (size_t)(hIt - _str.begin());
				};
			};
		};

		return _longestMatch;
	};
	constexpr auto find_longest_substr(auto&& _strings, const auto& _str)
	{
		if (_strings.empty()) { return _strings.end(); };
		return std::ranges::max_element(_strings, [_str](auto& lhs, auto& rhs) -> bool
			{
				return longest_substr(lhs, _str) < longest_substr(rhs, _str);
			});
	};


	constexpr std::string_view longest_match(const std::string_view _str, const std::string_view _substr)
	{
		if (_substr.empty()) { return 0; };

		auto sIt = _substr.begin();
		size_t _currentMatchLength = 0;
		for (auto hIt = _str.begin(); hIt != _str.end(); ++hIt)
		{
			if (*hIt == *sIt)
			{
				// start or continuation of match
				++_currentMatchLength;
				if (sIt == _substr.end())
				{
					// longest possible match found
					break;
				};
				++sIt; // next substr char
			}
			else
			{
				break;
			};
		};
		return _str.substr(0, _currentMatchLength);
	};
	constexpr auto find_longest_match(auto&& _strings, const auto& _prefix)
	{
		if (_strings.empty()) { return _strings.end(); };
		auto it = std::ranges::max_element(_strings, [_prefix](auto& lhs, auto& rhs) -> bool
			{
				return longest_match(lhs, _prefix) < longest_match(rhs, _prefix);
			});

		if (longest_match(*it, _prefix).empty())
		{
			return _strings.end();
		}
		else
		{
			return it;
		};
	};



	namespace impl
	{
		inline void concat_to_string_helper(std::stringstream& _sstr, const auto& v)
		{
			_sstr << v;
		};
	};

	inline std::string concat_to_string(const auto&... _parts)
	{
		auto ss = std::stringstream();
		(impl::concat_to_string_helper(ss, _parts), ...);
		return ss.str();
	};
};
