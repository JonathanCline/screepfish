#include "env.hpp"


#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <iostream>



sch::EnvInfo sch::load_env(const std::string& _executablePathStr)
{
	namespace fs = std::filesystem;

	const auto _executablePath = fs::path(_executablePathStr);
	const auto _executableDirectory = _executablePath.parent_path();
	const auto _envDirectory = _executableDirectory / "env";

	if (!fs::exists(_envDirectory) || !fs::is_directory(_envDirectory))
	{
		std::cout << "Missing env directory, expected at path " << _envDirectory.generic_string() << '\n';
		exit(1);
	};

	const auto _tokenFilePath = _envDirectory / "lichess_token.txt";
	if (!fs::exists(_tokenFilePath) || !fs::is_regular_file(_tokenFilePath))
	{
		std::cout << "Missing env/lichess_token.txt file, expected at path " << _tokenFilePath.generic_string() << '\n';
		exit(1);
	};

	auto _tokenFile = std::ifstream(_tokenFilePath);
	auto _token = std::string();
	std::getline(_tokenFile, _token);
	_tokenFile.close();

	if(_token.empty())
	{
		std::cout << "env/lichess_token.txt file missing token, add your lichess token at the top of the file. Path = " << _tokenFilePath.generic_string() << '\n';
		exit(1);
	};

	auto _info = sch::EnvInfo();
	_info.executable_path = _executablePath.generic_string();
	_info.executable_root_path = _executableDirectory.generic_string();
	_info.env_root_path = _envDirectory.generic_string();
	_info.token = _token;

	return _info;
};


