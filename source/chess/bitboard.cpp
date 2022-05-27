#include "bitboard.hpp"

#include <ostream>

namespace chess
{
	std::ostream& operator<<(std::ostream& _ostr, const BitBoard& _value)
	{
		_ostr << "+-+-+-+-+-+-+-+-+\n";
		for (auto& r : rev_ranks_v)
		{
			for (auto& f : files_v)
			{
				_ostr.put('|');
				if (_value.test(f, r))
				{
					_ostr.put('x');
				}
				else
				{
					_ostr.put(' ');
				};
			};
			_ostr.put('|');
			_ostr.put('\n');
			_ostr << "+-+-+-+-+-+-+-+-+\n";
		};
		return _ostr;
	};

};
