#include "system.hpp"

#define NOMINMAX
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <shellapi.h>

namespace sch
{
	bool open_browser(const char* _url)
	{
		if (const auto _result = ShellExecuteA(0, NULL, _url, 0, 0, SW_SHOW); _result > reinterpret_cast<HINSTANCE>(32))
		{
			return true;
		}
		else
		{
			return false;
		};
	};
}