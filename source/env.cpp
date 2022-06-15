#include "env.hpp"

#include "utility/string.hpp"
#include "utility/utility.hpp"
#include "utility/logging.hpp"

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <filesystem>



struct QueryErrorMessages
{
	std::string_view no_token_given = " No token given...";

	QueryErrorMessages() = default;
};

inline std::string query_user_for_string(std::string_view _prompt,
	QueryErrorMessages _errorMessages = QueryErrorMessages{})
{
	// Query the user for their lichess account token
	bool _keepQuerying = true;
	while (_keepQuerying)
	{
		auto _gotToken = std::string();
		std::cin.clear();
		std::cout << _prompt << ':';
		std::getline(std::cin, _gotToken);
		std::cin.clear();
		std::cout << '\n';

		if (_gotToken.empty())
		{
			// Query again
			std::cout << _errorMessages.no_token_given << '\n';
			continue;
		};

		return _gotToken;
	};
};

inline bool query_user_for_yesorno(std::string_view _prompt,
	QueryErrorMessages _errorMessages = QueryErrorMessages{})
{
	// Query the user for their lichess account token
	bool _keepQuerying = true;
	while (_keepQuerying)
	{
		auto _gotToken = std::string();
		std::cin.clear();
		std::cout << _prompt << " (Y/n) :";
		std::getline(std::cin, _gotToken);
		std::cin.clear();
		std::cout << '\n';

		if (_gotToken.find_first_of("ynYN") != 0)
		{
			// Query again
			std::cout << " Invalid input, expected one of 'y', 'n', 'Y', 'n'\n";
			continue;
		};

		if (_gotToken.starts_with("yY") == 0)
		{
			return true;
		}
		else if (_gotToken.starts_with("nN") == 0)
		{
			return false;
		};
	};
};



sch::EnvInfo sch::load_env(const std::string& _executablePathStr, bool _allowUserQuery)
{
	namespace fs = std::filesystem;

	const auto _executablePath = fs::canonical(fs::path(_executablePathStr));
	SCREEPFISH_CHECK(fs::exists(_executablePath));

	// Executable directory path
	const auto _executableDirectoryPath = fs::canonical(_executablePath).parent_path();
	SCREEPFISH_CHECK(fs::is_directory(_executableDirectoryPath));

	// Env directory handling
	const auto _envDirectoryPath = _executableDirectoryPath / "env";
	if (!fs::exists(_envDirectoryPath))
	{
		sch::log_info("Creating env directory at path " + _envDirectoryPath.generic_string());
		fs::create_directory(_envDirectoryPath);
		if (!fs::is_directory(_envDirectoryPath))
		{
			sch::log_error("Failed to create env directory at path " + _envDirectoryPath.generic_string());
			exit(1);
		};
	}
	else if (!fs::is_directory(_envDirectoryPath))
	{
		sch::log_error("Evaluated env directory path doesn't point to a directory! Expected at path " + _envDirectoryPath.generic_string());
		exit(1);
	};


	// Lichess account token string.
	std::string _lichessAccountToken{};

	// Token file path handling
	const auto _tokenFilePath = _envDirectoryPath / "lichess_token.txt";
	if (!fs::exists(_tokenFilePath))
	{
		if (_allowUserQuery)
		{
			_lichessAccountToken = query_user_for_string("Enter Lichess Account Token");
			bool _saveToFile = query_user_for_yesorno("Save To File");
			if (_saveToFile)
			{
				{
					auto _file = std::ofstream(_tokenFilePath);
					_file << _lichessAccountToken << '\n';
				};

				if (!fs::exists(_tokenFilePath))
				{
					sch::log_error("Failed to write account token to env file at path " + _tokenFilePath.generic_string());
					exit(1);
				};
			};
		}
		else
		{
			sch::log_error("Missing account token env file expected at path " + _tokenFilePath.generic_string());
			exit(1);
		};
	}
	else if (!fs::is_regular_file(_tokenFilePath))
	{
		sch::log_warning(str::concat_to_string("Token file path points to a non-text file at path ", _tokenFilePath.generic_string()));
		_lichessAccountToken = query_user_for_string("Enter Lichess Account Token");
	};

	// Load token from cached file if not already grabbed
	if (_lichessAccountToken.empty())
	{
		auto _tokenFile = std::ifstream(_tokenFilePath);
		std::getline(_tokenFile, _lichessAccountToken);
	};

	// No token :(
	if (_lichessAccountToken.empty())
	{
		sch::log_error(
			"Account token could not be parsed, add your lichess token at the top of the file. Path " +
			_tokenFilePath.generic_string()
		);
		exit(1);
	};

	// Set values
	auto _info = sch::EnvInfo();
	_info.executable_path = _executablePath.generic_string();
	_info.executable_root_path = _executableDirectoryPath.generic_string();
	_info.env_root_path = _envDirectoryPath.generic_string();
	_info.token = _lichessAccountToken;

	return _info;
};


