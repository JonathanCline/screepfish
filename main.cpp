#include "chess/fen.hpp"
#include "chess/chess.hpp"

#include "engine/engine.hpp"

#include "env.hpp"

#include "lichess/lichess.hpp"

#include "utility/string.hpp"
#include "utility/system.hpp"

#include <list>
#include <chrono>
#include <cassert>
#include <optional>
#include <iostream>

#include <jclib/functor.h>
#include <sstream>


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

		if (_event.status != "created" && _event.status != "started")
		{
			std::cout << "Result " << _event.status << '\n';
			return;
		};

		auto _board = chess::Board();
		chess::reset_board(_board);

		for (auto& _move : _moves)
		{
			_board.move_piece(_move.from(), _move.to(), _move.promotion());
		};

		class LichessGame : public chess::IGame
		{
		public:

			LichessGame() = default;
		};

		auto _game = LichessGame();
		_game.board_ = _board;
		this->engine_.set_board(_board);

		if (*this->my_turn_)
		{
			const auto _response = this->engine_.play_turn(_game);
			bool _passed = false;
			if (_response)
			{
				auto _params = lichess::MoveParams();
				_params.gameID = this->game_id_;
				std::stringstream sstr{};
				sstr << *_response;
				_params.move = sstr.str();
				_passed = this->client_.bot_move(_params).has_value();
			};
			if (!_passed)
			{
				auto _resignParams = lichess::ResignParams();
				_resignParams.gameID = this->game_id_;

				if (_response)
				{
					std::cout << "[ERROR] Failed to submit move : " << *_response << '\n';
					std::cout << _board << '\n';
					std::cout << chess::get_fen(_board) << '\n';
				};

				this->client_.bot_resign(_resignParams);
			};
		};
	};

	void on_game_full(const lichess::GameFullEvent& _event)
	{
		const auto lck = std::unique_lock(this->mtx_);

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

		// Tell the engine which color it is playing as
		this->engine_.set_color(*this->my_color_);

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
		const auto lck = std::unique_lock(this->mtx_);

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

	bool keep_open() const
	{
		const auto lck = std::unique_lock(this->mtx_);
		return this->keep_open_;
	};
	void set_close()
	{
		this->keep_open_ = false;
	};

	GameStream(const char* _token,
		const std::string& _gameID,
		const std::string& _playerID) :
		stream_(_token, "/api/bot/game/stream/" + _gameID),
		proc_(),
		player_id_(_playerID),
		game_id_(_gameID),
		client_(_token)
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

	mutable std::mutex mtx_;

	lichess::Client client_;
	lichess::StreamClient stream_;
	lichess::GameEventProcessor proc_;
	
	std::string game_id_;
	std::string player_id_;

	std::optional<chess::Color> my_color_{ std::nullopt };
	std::optional<bool> my_turn_{ std::nullopt };

	sch::ScreepFish engine_{};

	bool keep_open_ = true;
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
		
		{
			const auto _url = "https://lichess.org/" + _event.id;
			sch::open_browser(_url.c_str());
		};

		std::cout << "Started game " << _event.id << '\n';

		// Create the new stream
		this->game_streams_.emplace_back(this->env_.token.c_str(), _event.id, this->account_info_.id);
	};
	void game_finish_callback(const lichess::GameFinishEvent& _event)
	{
		const auto lck = std::unique_lock(this->mtx_);
		auto it = std::ranges::find_if(this->game_streams_, [&_event](const GameStream& v)
			{
				return v.game_id() == _event.id;
			});
		if (it != this->game_streams_.end())
		{
			it->set_close();
		};
		std::cout << "Finished game " << _event.id << '\n';
	};
	void challenge_callback(const lichess::ChallengeEvent& _event)
	{
		auto _params = lichess::AcceptChallengeParams();
		_params.challengeId = _event.id;
		this->account_client_.accept_challenge(_params);
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
		this->account_event_proc_.set_callback(jc::functor(&AccountManager::challenge_callback, this));
		this->account_event_stream_.start();

		{
			auto _games = this->account_client_.get_ongoing_games();
			for (auto& v : _games->nowPlaying)
			{
				this->game_streams_.emplace_back(this->env_.token.c_str(), v.gameId, this->account_info_.id);
			};

			if (_games->nowPlaying.empty())
			{
				using namespace std::chrono_literals;
			
				auto _params = lichess::ChallengeAIParams{};
				_params.level = 3;
				_params.days.reset();
				_params.clock.emplace();
				_params.clock->set_initial(1min);
				_params.clock->set_increment(4s);
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
	void update()
	{
		const auto lck = std::unique_lock(this->mtx_);
		std::erase_if(this->game_streams_, [](auto& v)
			{
				return !v.keep_open();
			});
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


bool run_tests()
{
	bool _runOnFinish = true;
	


	using namespace chess;

	{
		const auto _fen = "r3k1nr/pppn1ppp/4b3/4q3/Pb5P/8/3PP1P1/RNBQKBNR w KQkq - 0 8";
		auto _board = *parse_fen(_fen);
		_board.move(Move((File::d, Rank::r2), (File::d, Rank::r4)));
		if (!is_check(_board, Color::white))
		{
			abort();
		};
	};

	{
		const auto _fen = "1rb1kbnr/ppNppppp/2n5/6NQ/4P3/3P4/PPP2PPq/R3KB1R b KQk - 1 11";
		const auto _board = *parse_fen(_fen);
		
		std::cout << _fen << '\n';
		std::cout << get_fen(_board) << '\n';
		std::cout << _board << '\n';
		if (!is_check(_board, Color::black))
		{
			abort();
		};
	};

	{
		auto _board = Board();
		reset_board(_board);

		auto _tree = MoveTree();
		_tree.board = _board;
		_tree.to_play = _board.get_toplay();
		
		_tree.evalulate_next();
		if (const auto s = _tree.total_outcomes(); s != 20)
		{
			std::cout << "Expected 20, got " << s << '\n';
			abort();
		};

		_tree.evalulate_next();
		if (const auto s = _tree.total_outcomes(); s != 400)
		{
			std::cout << "Expected 400, got " << s << '\n';
			abort();
		};

		_tree.evalulate_next();
		if (const auto s = _tree.total_outcomes(); s != 8902)
		{
			std::cout << "Expected 8902, got " << s << '\n';
			std::cout << "  delta = " << (int)s - 8902 << '\n';
			//abort();
		};

		_tree.evalulate_next();
		if (const auto s = _tree.total_outcomes(); s != 197281)
		{
			std::cout << "Expected 197281, got " << s << '\n';
			std::cout << "  delta = " << (int)s - 8902 << '\n';
			//abort();
		};

	};




	return _runOnFinish;
};








int main(int _nargs, const char* _vargs[])
{
	if (!run_tests()) { return 0; };

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
		_accountManager.update();
	};

	return 0;
};