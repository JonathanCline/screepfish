#pragma once

/** @file */

#include <string>

namespace sch
{
	struct EnvInfo
	{
		std::string executable_path;
		std::string executable_root_path;
		std::string env_root_path;
		std::string token;

		EnvInfo() = default;
	};

	EnvInfo load_env(const std::string& _executablePath, bool _allowUserQuery = false);

};
