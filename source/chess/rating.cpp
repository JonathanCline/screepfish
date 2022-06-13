#include "rating.hpp"

#include <ostream>

namespace chess
{
	std::ostream& operator<<(std::ostream& _ostr, const AbsoluteRating& _value)
	{
		return _ostr << _value.raw() << " aRT";
	};
};