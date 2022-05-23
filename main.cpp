#include "chess/chess.hpp"

#include "env.hpp"

#include "lichess/lichess.hpp"

#include "utility/string.hpp"

#include <list>
#include <chrono>
#include <cassert>
#include <optional>
#include <iostream>

#include <jclib/functor.h>




struct GameStream
{
private:

	void on_move_played(const lichess::GameStateEvent& _event)
	{
		std::vector<chess::Move> _moves{};

		{
			auto _movesString = std::string_view(_event.moves);
			while (!_movesString.empty())
			{
				auto _move = chess::Move();
				_movesString = chess::fromstr(_movesString, _move);
				if (!_movesString.empty())
				{
					_movesString = str::lstrip(_movesString);
				};
				_moves.push_back(_move);
			};
		};

		std::cout << "moves : \n";
		for (auto& v : _moves)
		{
			std::cout << ' ' << v << '\n';
		};
		std::cout << '\n';

	};

	void on_game_full(const lichess::GameFullEvent& _event)
	{
		if (auto& _blackID = _event.black.id; _blackID && _blackID.value() == this->player_id_)
		{
			this->my_turn_ = false;
			this->my_color_ = chess::Color::black;
		}
		else if (auto& _whiteID = _event.white.id; _whiteID && _whiteID.value() == this->player_id_)
		{
			this->my_turn_ = true;
			this->my_color_ = chess::Color::white;
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

		if (this->my_color_.value() == chess::Color::white && _whiteToPlay)
		{
			this->my_turn_ = true;
		}
		else if (this->my_color_.value() == chess::Color::black && !_whiteToPlay)
		{
			this->my_turn_ = true;
		}
		else
		{
			this->my_turn_ = false;
		};

		this->on_move_played(_event.state);
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

		if (this->my_color_.value() == chess::Color::white && _whitesTurn)
		{
			this->my_turn_ = true;
		}
		else if (this->my_color_.value() == chess::Color::black && !_whitesTurn)
		{
			this->my_turn_ = true;
		}
		else
		{
			this->my_turn_ = false;
		};

		this->on_move_played(_event);
	};

public:

	std::string_view game_id() const & noexcept
	{
		return this->game_id_;
	};
	std::string_view game_id() && = delete;


	GameStream(const char* _token, const std::string& _gameID, const std::string& _playerID) :
		stream_(_token, "/api/bot/game/stream/" + _gameID),
		proc_(),
		player_id_(_playerID),
		game_id_(_gameID)
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
	
	std::string game_id_;
	std::string player_id_;

	std::optional<chess::Color> my_color_{ std::nullopt };
	std::optional<bool> my_turn_{ std::nullopt };
};


struct AccountManager
{
private:

	void game_start_callback(const lichess::GameStartEvent& _event)
	{
		const auto lck = std::unique_lock(this->mtx_);
		for (auto& v : this->game_streams_)
		{
			if (v.game_id() == _event.id)
			{
				std::cout << "[WARNING] Got game start event for a game we are already managing\n";
				return;
			};
		};
		
		std::cout << "Started game " << _event.id << '\n';

		// Create the new stream
		this->game_streams_.emplace_back(this->env_.token.c_str(), _event.id, this->account_info_.id);
	};
	void game_finish_callback(const lichess::GameFinishEvent& _event)
	{
		const auto lck = std::unique_lock(this->mtx_);
		std::erase_if(this->game_streams_, [&_event](const GameStream& v)
			{
				return v.game_id() == _event.id;
			});

		std::cout << "Finished game " << _event.id << '\n';
	};

public:

	void start()
	{
		this->account_info_ = *this->account_client_.get_account_info();
		
		// Lock while we perform setup
		const auto lck = std::unique_lock(this->mtx_);

		// Setup account event stream
		this->account_event_stream_.set_callback(jc::functor(&lichess::AccountEventProcessor::process, &this->account_event_proc_));
		this->account_event_proc_.set_callback(jc::functor(&AccountManager::game_start_callback, this));
		this->account_event_proc_.set_callback(jc::functor(&AccountManager::game_finish_callback, this));
		this->account_event_stream_.start();


		{
			bool _hasAIGame = false;
			auto _games = this->account_client_.get_ongoing_games();
			for (auto& v : _games->nowPlaying)
			{
				if (v.opponent.ai)
				{
					_hasAIGame = true;
				};
				const auto _endpoint = "/api/bot/game/stream/" + v.gameId;
				this->game_streams_.emplace_back(this->env_.token.c_str(), _endpoint, this->account_info_.id);
			};

			if (!_hasAIGame)
			{
				auto _params = lichess::ChallengeAIParams{};
				auto _result = this->account_client_.challenge_ai(_params);
			};
		};

		{
			auto _challenges = this->account_client_.get_challenges();
			for (auto& v : _challenges->in)
			{
				if (v.status == "created")
				{
					auto _params = lichess::AcceptChallengeParams();
					_params.challengeId = v.id;
					if (!this->account_client_.accept_challenge(_params))
					{
						std::cout << "[ERROR] Failed to accept challenge with ID " <<
							v.id << '\n';
					};
				};
			};
		};
	};

	AccountManager(sch::EnvInfo _env) :
		env_(std::move(_env)),
		account_client_(this->env_.token.c_str()),
		account_event_stream_(this->env_.token.c_str(), "/api/stream/event"),
		account_event_proc_()
	{};

private:
	mutable std::mutex mtx_;

	sch::EnvInfo env_;
	lichess::Client account_client_;
	lichess::AccountInfo account_info_;

	lichess::StreamClient account_event_stream_;
	std::list<GameStream> game_streams_{};

	lichess::AccountEventProcessor account_event_proc_;
};


int main(int _nargs, const char* _vargs[])
{
	if (_nargs == 0 || !_vargs || !_vargs[0])
	{
		std::cout << "No arguments provided, not even exec path1!\n";
		exit(1);
	};

	const auto _env = sch::load_env(_vargs[0]);
	auto _accountManager = AccountManager(_env);
	_accountManager.start();

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	};

	return 0;
};