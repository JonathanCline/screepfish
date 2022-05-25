#include "position.hpp"

#include <ostream>
#include <istream>

namespace chess
{
	std::ostream& operator<<(std::ostream& _ostr, const File& _value)
	{
		return _ostr << tochar(_value);
	};
	std::istream& operator>>(std::istream& _istr, File& _value)
	{
		const auto c = _istr.peek();
		if (c >= static_cast<int>('a') && c <= static_cast<int>('h'))
		{
			fromchar(static_cast<char>(c), _value);
			_istr.ignore();
		}
		else
		{
			_istr.setstate(std::ios_base::failbit);
		};
		return _istr;
	};

	std::ostream& operator<<(std::ostream& _ostr, const Rank& _value)
	{
		return _ostr << tochar(_value);
	};
	std::istream& operator>>(std::istream& _istr, Rank& _value)
	{
		const auto c = _istr.peek();
		if (c >= static_cast<int>('1') && c <= static_cast<int>('8'))
		{
			fromchar(static_cast<char>(c), _value);
			_istr.ignore();
		}
		else
		{
			_istr.setstate(std::ios_base::failbit);
		};
		return _istr;
	};

	std::ostream& operator<<(std::ostream& _ostr, const Position& _value)
	{
		return _ostr << _value.file() << _value.rank();
	};

};
