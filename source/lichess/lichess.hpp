#pragma once

/** @file */

#include "json.hpp"

#include <jclib/maybe.h>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
	#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include <httplib.h>

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <thread>
#include <mutex>
#include <functional>
#include <latch>
#include <chrono>

namespace lichess
{
	struct AccountInfo
	{
		std::string id;
		std::string username;
		bool online;

		AccountInfo() = default;
	};

	struct OngoingGame
	{
		struct Opponent
		{
			std::optional<int> ai;
			std::optional<std::string> id;
			std::string username;
			std::optional<uint32_t> rating;
		};

		std::string gameId;
		std::string fullId;
		std::string color;
		std::string fen;
		std::string source;
		std::string lastMove;
		std::optional<uint64_t> secondsLeft;
		bool rated;
		bool hasMoved;
		bool isMyTurn;
		Opponent opponent;

		OngoingGame() = default;
	};

	struct OngoingGames
	{
		std::vector<OngoingGame> nowPlaying;

		OngoingGames() = default;
	};

	struct Challenge
	{
		struct Challenger
		{
			std::string id;
			std::string name;
			std::optional<std::string> title;
			int32_t rating;
			std::optional<bool> online;

			Challenger() = default;
		};

		struct TimeControl
		{
			std::string type;
			std::optional<std::string> show;
			std::optional<int64_t> limit;
			std::optional<int64_t> increment;

			TimeControl() = default;
		};

		struct Variant
		{
			std::string key;
			std::string name;
			Variant() = default;
		};

		std::string id;
		std::string url;
		std::string color;
		std::optional<std::string> direction;
		TimeControl timeControl;
		Challenger challenger;
		Challenger destUser;
		std::string speed;
		std::string status;
		Variant variant;
		bool rated;
		
		Challenge() = default;
	};

	struct Challenges
	{
		std::vector<Challenge> in;
		std::vector<Challenge> out;

		Challenges() = default;
	};


	/**
	 * Params type.
	 * 
	 * https://lichess.org/api#operation/challengeAi
	*/
	struct ChallengeAIParams
	{
		uint8_t level = 1;
		std::optional<uint8_t> days = 1;
		std::string color = "random";
		std::string variant = "standard";
		std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

		struct Clock
		{
			Clock& set_initial(std::chrono::seconds _time)
			{
				this->limit_ = _time.count();
				return *this;
			};
			Clock& set_increment(std::chrono::seconds _time)
			{
				this->increment_ = _time.count();
				return *this;
			};

			template <typename Rep, typename Period>
			auto& set_initial(std::chrono::duration<Rep, Period> _time)
			{
				return this->set_initial(std::chrono::duration_cast<std::chrono::seconds>(_time));
			};
			template <typename Rep, typename Period>
			auto& set_increment(std::chrono::duration<Rep, Period> _time)
			{
				return this->set_increment(std::chrono::duration_cast<std::chrono::seconds>(_time));
			};

			int64_t limit_ = 3000;
			int64_t increment_ = 0;

			Clock() = default;
		};
		std::optional<Clock> clock;

		ChallengeAIParams() = default;
	};

	/**
	 * Result type.
	 * 
	 * https://lichess.org/api#operation/challengeAi
	*/
	struct ChallengeAI
	{
		std::string id;
		bool rated;
		std::string variant;
		std::string speed;
		std::string perf;
		int64_t createdAt;
		int64_t lastMoveAt;
		std::string status;
		
		ChallengeAI() = default;
	};




	struct GameStartEvent
	{
		std::string id;
		std::string source;

		GameStartEvent() = default;
	};

	struct GameFinishEvent
	{
		std::string id;
		std::string source;

		GameFinishEvent() = default;
	};

	using ChallengeEvent = Challenge;


	struct AcceptChallengeParams
	{
		std::string challengeId;

		AcceptChallengeParams() = default;
	};
	struct AcceptChallenge
	{
		bool ok;

		AcceptChallenge() = default;
	};





	struct AccountEventProcessor
	{
	public:

		using GameStartCallback = std::function<void(const GameStartEvent&)>;
		using GameFinishCallback = std::function<void(const GameFinishEvent&)>;
		using ChallengeCallback = std::function<void(const ChallengeEvent&)>;

		void set_callback(GameStartCallback _callback)
		{
			const auto lck = std::unique_lock(this->mtx_);
			this->game_start_callback_ = std::move(_callback);
		};
		void set_callback(GameFinishCallback _callback)
		{
			const auto lck = std::unique_lock(this->mtx_);
			this->game_finish_callback_ = std::move(_callback);
		};
		void set_callback(ChallengeCallback _callback)
		{
			const auto lck = std::unique_lock(this->mtx_);
			this->challenge_callback_ = std::move(_callback);
		};

		void push(const GameStartEvent& _event) const;
		void push(const GameFinishEvent& _event) const;
		void push(const ChallengeEvent& _event) const;

		void process(const json& _json);

		AccountEventProcessor() = default;

	private:
		mutable std::mutex mtx_;
		GameStartCallback game_start_callback_;
		GameFinishCallback game_finish_callback_;
		ChallengeCallback challenge_callback_;
	};



	struct GameStateEvent
	{
		std::string moves;
		int64_t wtime;
		int64_t btime;
		int64_t winc;
		int64_t binc;
		std::string status;
		std::optional<std::string> winner;

		std::optional<bool> wdraw;
		std::optional<bool> bdraw;
		std::optional<bool> wtakeback;
		std::optional<bool> btakeback;

		GameStateEvent() = default;
	};

	struct GameFullEvent
	{
		struct Player
		{
			std::optional<uint8_t> aiLevel;
			std::optional<std::string> id;
			std::optional<std::string> name;
			std::optional<std::string> title;
			std::optional<int32_t> rating;
			std::optional<bool> provisional;

			Player() = default;
		};
		
		struct Clock
		{
			int64_t initial;
			int64_t increment;

			Clock() = default;
		};

		std::string id;
		std::optional<Clock> clock;
		std::string speed;
		bool rated;
		int64_t createdAt;
		Player white;
		Player black;

		std::string initialFen;
		GameStateEvent state;

		GameFullEvent() = default;
	};



	struct GameEventProcessor
	{
	public:
		using GameFullCallback = std::function<void(const GameFullEvent&)>;
		using GameStateCallback = std::function<void(const GameStateEvent&)>;

		void set_callback(GameFullCallback _callback)
		{
			const auto lck = std::unique_lock(this->mtx_);
			this->game_full_callback_ = std::move(_callback);
		};
		void set_callback(GameStateCallback _callback)
		{
			const auto lck = std::unique_lock(this->mtx_);
			this->game_state_callback_ = std::move(_callback);
		};

		void push(const GameFullEvent& _event) const;
		void push(const GameStateEvent& _event) const;

		void process(const json& _json);

		GameEventProcessor() = default;
		
	private:		
		mutable std::mutex mtx_;
		GameFullCallback game_full_callback_;
		GameStateCallback game_state_callback_;
	};


	struct MoveParams
	{
		std::string gameID;
		std::string move;
		std::optional<bool> offeringDraw;
		MoveParams() = default;
	};

	struct Move
	{
		bool ok;
		Move() = default;
	};

	struct ResignParams
	{
		std::string gameID;
		ResignParams() = default;
	};

	struct Resign
	{
		bool ok;
		Resign() = default;
	};



	struct FailResult
	{

		FailResult() = default;
		
		explicit FailResult(httplib::Error _error, int _status) :
			error_(_error), status_(_status)
		{};
		explicit FailResult(int _status) :
			error_{}, status_(_status)
		{};


		httplib::Error error_{};
		int status_ = 0;
	};

	template <typename T>
	using Result = jc::maybe<T, FailResult>;



	class StreamClient
	{
	public:
		using CallbackFn = std::function<void(const json&)>;

	private:
		
		struct Data
		{
			std::mutex mtx;
			httplib::Client client;
			CallbackFn callback;
			std::latch init_latch{ 2 };
			std::string endpoint;
			std::string path{};

			Data(const char* _authToken, std::string _endpoint) :
				client("https://lichess.org"),
				endpoint(_endpoint)
			{
				this->client.set_bearer_token_auth(_authToken);
			};
		};

		/**
		 * @brief Content reciever function for new line delimeted json data
		 * @param _data Pointer to the received data
		 * @param _len Length of the data
		 * @return True if content is valid, false otherwise
		*/
		static bool ndjson_content_reciever(const char* _data, size_t _len);

		static void thread_main(std::stop_token _stop, std::shared_ptr<Data> _data)
		{
			auto& _client = _data->client;
			auto& _mtx = _data->mtx;
			auto& _callback = _data->callback;
			auto _path = _data->endpoint;

			_data->init_latch.arrive_and_wait();

			
			std::cout << "[Debug] Entered main stream loop" << std::endl;

			auto _headers = httplib::Headers();
			_client.set_read_timeout(std::chrono::minutes{ 2 });
			_client.set_write_timeout(std::chrono::minutes{ 2 });
			_client.set_connection_timeout(std::chrono::seconds(20));
			_client.set_keep_alive(true);
			const auto _isSocketOpen = _client.is_socket_open();
			std::cout << _isSocketOpen << std::endl;
			const auto _isValid = _client.is_valid();
			std::cout << _isValid << std::endl;

			auto& _loggingPath = _data->path;
			auto _result = _client.Get(_path.c_str(), _headers,
				[&_stop](const httplib::Response& _response) -> bool
				{
					if (_stop.stop_requested())
					{
						std::cout << "[Debug] Stop requested" << std::endl;
						return false;
					}
					else
					{
						return true;
					};
				},
				[&](const char* _data, size_t _len) -> bool
				{
					if (_len > 1)
					{
						auto _node = json::parse(std::string_view{ _data, _len }, nullptr, false);
						if (!_node.is_discarded())
						{
							if (!_loggingPath.empty())
							{
								auto _file = std::ofstream(_loggingPath, std::ios::out | std::ios::app);
								_file << _node.dump(1, '\t') << std::endl;
							};

							const auto lck = std::unique_lock(_mtx);
							if (_callback)
							{
								_callback(_node);
							};
						};
					}
					else
					{
						json _json{ { "still-alive", true } };
						const auto lck = std::unique_lock(_mtx);
						if (_callback)
						{
							_callback(_json);
						};
					};
					
					if (_stop.stop_requested())
					{
						std::cout << "[Debug] Stop requested" << std::endl;
						return false;
					}
					else
					{
						return true;
					};
				}
				);

			std::cout << "[Debug] Exiting main stream loop" << std::endl;

			if (_result.error() == httplib::Error::Success)
			{
				// good
			}
			else
			{
				std::cout << "[Error] Stream exited with error " << (int)_result.error() << '\n';
			};
		};

	public:

		void set_callback(CallbackFn _fn)
		{
			const auto lck = std::unique_lock(this->data_->mtx);
			this->data_->callback = std::move(_fn);
		};

		void start()
		{
			this->thread_ = std::jthread(&thread_main, this->data_);
			this->data_->init_latch.arrive_and_wait();
		};

		void enable_logging(const std::string& _path);


		StreamClient(const char* _authToken, std::string _endpoint) :
			data_(new Data(_authToken, _endpoint))
		{};

		StreamClient(StreamClient&& other) noexcept :
			data_(other.data_),
			thread_(std::move(other.thread_))
		{
			other.data_.reset();
		}
		StreamClient& operator=(StreamClient&& other) noexcept
		{
			if (this->data_ && this->data_->client.is_socket_open())
			{
				this->data_->client.stop();
			};
			this->thread_.request_stop();
			this->thread_.join();
			this->data_.reset();

			this->data_ = other.data_;
			this->thread_ = std::move(other.thread_);

			other.data_.reset();
			return *this;
		};

		~StreamClient()
		{
			if (this->data_ && this->data_->client.is_socket_open())
			{
				this->data_->client.stop();
			};
		};

	private:
		std::shared_ptr<Data> data_;
		std::jthread thread_;
	};

	class Client
	{
	public:

		Result<AccountInfo> get_account_info();
		Result<OngoingGames> get_ongoing_games();
		Result<Challenges> get_challenges();

		Result<ChallengeAI> challenge_ai(ChallengeAIParams _params);
		Result<AcceptChallenge> accept_challenge(AcceptChallengeParams _params);

		Result<Move> bot_move(MoveParams _params);
		Result<Resign> bot_resign(ResignParams _params);


		Client(const char* _authToken);

	private:
		httplib::Client client_;
		httplib::Client opening_explorer_client_;
	};

};
