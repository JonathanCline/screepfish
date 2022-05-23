#include "env.hpp"

#include "lichess/lichess.hpp"

#include <iostream>
#include <cassert>
#include <optional>
#include <chrono>

int main(int _nargs, const char* _vargs[])
{
	if (_nargs == 0 || !_vargs || !_vargs[0])
	{
		std::cout << "No arguments provided, not even exec path1!\n";
		exit(1);
	};

	const auto _env = sch::load_env(_vargs[0]);
	auto _client = lichess::Client(_env.token.c_str());


	auto _eventStream = lichess::StreamClient(_env.token.c_str(),
		"/api/stream/event");
	_eventStream.set_callback([](const lichess::json& _json)
		{
			if (_json.contains("still-alive"))
			{
				return;
			};

			const std::string _type = _json.at("type");
			if (_type == "gameStart")
			{
				std::cout << "Game start " << _json.at("game").at("gameId") << '\n';
			}
			else if (_type == "gameFinish")
			{
				std::cout << "Game finish " << _json.at("game").at("gameId") << '\n';
			};
		});
	_eventStream.start();



	auto _gameStreams = std::vector<lichess::StreamClient>();

	bool _hasAIGame = false;

	{
		auto _games = _client.get_ongoing_games();
		for (auto& v : _games->nowPlaying)
		{
			std::cout << v.gameId << '\n';
			std::cout << v.opponent.username << '\n';

			if (v.opponent.ai)
			{
				_hasAIGame = true;
			};

			const auto _endpoint = "/api/bot/game/stream/" + v.gameId;
			_gameStreams.push_back(lichess::StreamClient(_env.token.c_str(), _endpoint));
			_gameStreams.back().set_callback([](const lichess::json& _json)
				{
					if (_json.contains("still-alive"))
					{
						return;
					};
				});
			_gameStreams.back().start();
		};
		std::cout << '\n';
	};
	
	{
		auto _challenges = _client.get_challenges();
		for (auto& v : _challenges->in)
		{
			if (v.status == "created")
			{
				auto _params = lichess::AcceptChallengeParams();
				_params.challengeId = v.id;
				if (!_client.accept_challenge(_params))
				{
					std::cout << "[ERROR] Failed to accept challenge with ID " <<
						v.id << '\n';
				};
			};
		};
	};

	if (!_hasAIGame)
	{
		auto _params = lichess::ChallengeAIParams{};
		auto _result = _client.challenge_ai(_params);
		std::cout << "Created game against AI at " << _result->id << '\n';
	};

	std::this_thread::sleep_for(std::chrono::seconds(30));

	return 0;
};