#include "env.hpp"

#include "lichess/lichess.hpp"

#include <list>
#include <chrono>
#include <cassert>
#include <optional>
#include <iostream>

#include <jclib/functor.h>





struct GameStream
{
private:

	void on_my_turn(const lichess::GameStateEvent& _event)
	{


		std::cout << "my turn " << _event.moves << '\n';
	};

	void on_game_full(const lichess::GameFullEvent& _event)
	{
		if (auto& _blackID = _event.black.id; _blackID && _blackID.value() == this->player_id_)
		{
			this->my_turn_ = false;
			this->my_color_ = "black";
		}
		else if (auto& _whiteID = _event.white.id; _whiteID && _whiteID.value() == this->player_id_)
		{
			this->my_turn_ = true;
			this->my_color_ = "white";
		};

		// Count played moves
		auto _movesPlayedCount = std::ranges::count(_event.state.moves, ' ');
		if (!_event.state.moves.empty())
		{
			_movesPlayedCount += 1;
		};

		// Set if its our turn or not
		bool _whiteToPlay = false;
		if ((_movesPlayedCount % 2) == 0)
		{
			_whiteToPlay = true;
		};

		if (this->my_color_.value() == "white" && _whiteToPlay)
		{
			this->my_turn_ = true;
		}
		else if (this->my_color_.value() == "black" && !_whiteToPlay)
		{
			this->my_turn_ = true;
		}
		else
		{
			this->my_turn_ = false;
		};

		if (this->my_turn_)
		{
			this->on_my_turn(_event.state);
		};
	};
	void on_game_state(const lichess::GameStateEvent& _event)
	{
		// Count played moves
		auto _movesPlayedCount = std::ranges::count(_event.moves, ' ');
		if (!_event.moves.empty())
		{
			_movesPlayedCount += 1;
		};

		bool _whitesTurn = false;
		if ((_movesPlayedCount % 2) == 0)
		{
			_whitesTurn = true;
		};

		if (this->my_color_.value() == "white" && _whitesTurn)
		{
			this->my_turn_ = true;
		}
		else if (this->my_color_.value() == "black" && !_whitesTurn)
		{
			this->my_turn_ = true;
		}
		else
		{
			this->my_turn_ = false;
		};

		if (this->my_turn_.value())
		{
			this->on_my_turn(_event);
		};
	};

public:

	GameStream(const char* _token, const std::string& _endpoint, const std::string& _playerID) :
		stream_(_token, _endpoint),
		proc_(),
		player_id_(_playerID)
	{
		this->proc_.set_callback(jc::functor(&GameStream::on_game_full, this));
		this->proc_.set_callback(jc::functor(&GameStream::on_game_state, this));

		this->stream_.set_callback([this](const lichess::json& _json)
			{
				this->proc_.process(_json);
			});
		this->stream_.start();
	};

private:
	lichess::StreamClient stream_;
	lichess::GameEventProcessor proc_;
	std::string player_id_;

	std::optional<bool> my_turn_{ std::nullopt };
	std::optional<std::string> my_color_{ std::nullopt };

};


int main(int _nargs, const char* _vargs[])
{
	if (_nargs == 0 || !_vargs || !_vargs[0])
	{
		std::cout << "No arguments provided, not even exec path1!\n";
		exit(1);
	};

	const auto _env = sch::load_env(_vargs[0]);
	auto _client = lichess::Client(_env.token.c_str());

	auto _accountInfo = _client.get_account_info();


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


	auto _gameStreams = std::list<GameStream>();

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
			_gameStreams.emplace_back(_env.token.c_str(), _endpoint, _accountInfo->id);
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

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	};

	return 0;
};