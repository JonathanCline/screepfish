#include "lichess.hpp"
#include "json.hpp"

#include <array>
#include <tuple>
#include <string_view>


#define _LICHESS_MEMBER(member) ::lichess::named_member(&type::member, #member)

namespace lichess
{
	namespace impl
	{
		template <typename T>
		requires requires (const T& v)
		{
			std::to_string(v);
		}
		inline auto tostr(const T& v)
		{
			return std::to_string(v);
		};

		inline auto tostr(uint8_t b)
		{
			return std::to_string(b);
		};
		inline auto tostr(uint16_t b)
		{
			return std::to_string(b);
		};
		inline auto tostr(uint32_t b)
		{
			return std::to_string(b);
		};
		inline auto tostr(uint64_t b)
		{
			return std::to_string(b);
		};

		inline auto tostr(int8_t b)
		{
			return std::to_string(b);
		};
		inline auto tostr(int16_t b)
		{
			return std::to_string(b);
		};
		inline auto tostr(int32_t b)
		{
			return std::to_string(b);
		};
		inline auto tostr(int64_t b)
		{
			return std::to_string(b);
		};

		inline auto tostr(bool b)
		{
			return (b) ? "true" : "false";
		};

		inline auto tostr(const std::string& v)
		{
			return v;
		};
		

		struct ToParamMemberOp
		{
			template <typename T>
			bool add_to_params(const T& _var) const
			{
				return true;
			};
			template <typename T>
			bool add_to_params(const std::optional<T>& _var) const
			{
				return _var.has_value();
			};


			template <typename T>
			std::string pstr(const T& _var) const
			{
				return tostr(_var);
			};
			template <typename T>
			std::string pstr(const std::optional<T>& _var) const
			{
				return pstr(_var.value());
			};
			
				
			template <typename T, typename V>
			void operator()(httplib::Params& _params, const T* _class, const named_member<T, V>& _info) const
			{
				auto& v = _info.get(_class);
				if (this->add_to_params(v))
				{
					_params.insert(std::make_pair(std::string(_info.name()), this->pstr(v)));
				};
			};

			void operator()(httplib::Params& _params, const ChallengeAIParams* _class, const named_member<ChallengeAIParams,
				std::optional<ChallengeAIParams::Clock>>& _info) const
			{
				auto& v = _info.get(_class);
				if (this->add_to_params(v))
				{
					_params.insert({ "clock.limit", this->pstr(v->limit_) });
					_params.insert({ "clock.increment", this->pstr(v->increment_) });
				};
			};
		};

		template <typename DestType, typename OpT>
		struct MemberProc
		{
			using dest_type = DestType;
			using op = OpT;

		//private:
			using this_type = MemberProc<DestType, OpT>;

			template <typename T, typename V>
			//requires requires (dest_type _dest, const T* _class, const named_member<T, V>& _info, op o)
			//{
			//	o(_dest, _class, _info);
			//}
			static void handle_member(dest_type _dest, const T* _class, const named_member<T, V>& _info)
			{
				op o{};
				o(_dest, _class, _info);
			};

			template <typename T, typename... Ts, size_t... Idxs>
			//requires requires (dest_type _dest, const T* _class, const std::tuple<Ts...>& _tup, std::index_sequence<Idxs...>)
			//{
			//	(this_type::handle_member(_dest, _class, std::get<Idxs>(_tup)), ...);
			//}
			static void handle_members_impl(dest_type _dest, const T* _class, const std::tuple<Ts...>& _tup, std::index_sequence<Idxs...>)
			{
				(this_type::handle_member(_dest, _class, std::get<Idxs>(_tup)), ...);
			};

			template <typename T, typename... Ts>
			//requires requires(dest_type _dest, const T* _class, const std::tuple<Ts...>& _tup)
			//{
			//	this_type::handle_members_impl(_dest, _class, _tup, std::make_index_sequence<sizeof...(Ts)>{});
			//}
			static void handle_members(dest_type _dest, const T* _class, const std::tuple<Ts...>& _tup)
			{
				return this_type::handle_members_impl(_dest, _class, _tup, std::make_index_sequence<sizeof...(Ts)>{});
			};

		public:

			template <typename T>
			requires requires(dest_type _dest, const T& _class)
			{
				this_type::handle_members(_dest, &_class, class_named_member_table_v<T>);
			}
			inline auto operator()(dest_type _dest, const T& _class) const
			{
				return this_type::handle_members(_dest, &_class, class_named_member_table_v<T>);
			};
		};

		using ToParamMemberProc = MemberProc<httplib::Params&, ToParamMemberOp>;
	};
};





namespace lichess
{
	template <>
	struct class_named_member_table<AccountInfo>
	{
		using type = AccountInfo;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(id),
			_LICHESS_MEMBER(online),
			_LICHESS_MEMBER(username)
		);
	};

	template <>
	struct class_named_member_table<OngoingGame::Opponent>
	{
		using type = OngoingGame::Opponent;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(id),
			_LICHESS_MEMBER(username),
			_LICHESS_MEMBER(rating),
			_LICHESS_MEMBER(ai)
		);
	};
	
	template <>
	struct class_named_member_table<OngoingGame>
	{
		using type = OngoingGame;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(gameId),
			_LICHESS_MEMBER(fullId),
			_LICHESS_MEMBER(color),
			_LICHESS_MEMBER(fen),
			_LICHESS_MEMBER(lastMove),
			_LICHESS_MEMBER(secondsLeft),
			_LICHESS_MEMBER(rated),
			_LICHESS_MEMBER(hasMoved),
			_LICHESS_MEMBER(isMyTurn),
			_LICHESS_MEMBER(opponent),
			_LICHESS_MEMBER(source)
		);
	};
	template <>
	struct class_named_member_table<OngoingGames>
	{
		using type = OngoingGames;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(nowPlaying)
		);
	};


	template <>
	struct class_named_member_table<Challenge::Challenger>
	{
		using type = Challenge::Challenger;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(id),
			_LICHESS_MEMBER(name),
			_LICHESS_MEMBER(title),
			_LICHESS_MEMBER(rating),
			_LICHESS_MEMBER(online)
		);
	};
	template <>
	struct class_named_member_table<Challenge::Variant>
	{
		using type = Challenge::Variant;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(key),
			_LICHESS_MEMBER(name)
		);
	};

	template <>
	struct class_named_member_table<Challenge::TimeControl>
	{
		using type = Challenge::TimeControl;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(type),
			_LICHESS_MEMBER(show),
			_LICHESS_MEMBER(increment),
			_LICHESS_MEMBER(limit)
		);
	};

	template <>
	struct class_named_member_table<Challenge>
	{
		using type = Challenge;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(id),
			_LICHESS_MEMBER(url),
			_LICHESS_MEMBER(color),
			_LICHESS_MEMBER(direction),
			_LICHESS_MEMBER(timeControl),
			_LICHESS_MEMBER(challenger),
			_LICHESS_MEMBER(destUser),
			_LICHESS_MEMBER(speed),
			_LICHESS_MEMBER(status),
			_LICHESS_MEMBER(variant),
			_LICHESS_MEMBER(rated)
		);
	};
	template <>
	struct class_named_member_table<Challenges>
	{
		using type = Challenges;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(in),
			_LICHESS_MEMBER(out)
		);
	};



	template <>
	struct class_named_member_table<ChallengeAIParams::Clock>
	{
		using type = ChallengeAIParams::Clock;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(increment_),
			_LICHESS_MEMBER(limit_)
		);
	};

	template <>
	struct class_named_member_table<ChallengeAIParams>
	{
		using type = ChallengeAIParams;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(level),
			_LICHESS_MEMBER(days),
			_LICHESS_MEMBER(color),
			_LICHESS_MEMBER(variant),
			_LICHESS_MEMBER(fen),
			_LICHESS_MEMBER(clock)
		);
	};


	template <>
	struct class_named_member_table<ChallengeAI>
	{
		using type = ChallengeAI;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(id),
			_LICHESS_MEMBER(rated),
			_LICHESS_MEMBER(variant),
			_LICHESS_MEMBER(speed),
			_LICHESS_MEMBER(perf),
			_LICHESS_MEMBER(createdAt),
			_LICHESS_MEMBER(lastMoveAt),
			_LICHESS_MEMBER(status)
		);
	};

	template <>
	struct class_named_member_table<AcceptChallenge>
	{
		using type = AcceptChallenge;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(ok)
		);
	};

	template <>
	struct class_named_member_table<GameStartEvent>
	{
		using type = GameStartEvent;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(id),
			_LICHESS_MEMBER(source)
		);
	};

	template <>
	struct class_named_member_table<GameFinishEvent>
	{
		using type = GameFinishEvent;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(id),
			_LICHESS_MEMBER(source)
		);
	};

	template <>
	struct class_named_member_table<GameStateEvent>
	{
		using type = GameStateEvent;
		constexpr static auto value = std::make_tuple
		(		
			_LICHESS_MEMBER(moves),
			_LICHESS_MEMBER(wtime),
			_LICHESS_MEMBER(btime),
			_LICHESS_MEMBER(winc),
			_LICHESS_MEMBER(binc),
			_LICHESS_MEMBER(status),
			_LICHESS_MEMBER(winner),
			_LICHESS_MEMBER(wdraw),
			_LICHESS_MEMBER(bdraw),
			_LICHESS_MEMBER(wtakeback),
			_LICHESS_MEMBER(btakeback)
		);
	};

	template <>
	struct class_named_member_table<GameFullEvent::Player>
	{
		using type = GameFullEvent::Player;
		constexpr static auto value = std::make_tuple
		(		
			_LICHESS_MEMBER(aiLevel),
			_LICHESS_MEMBER(id),
			_LICHESS_MEMBER(name),
			_LICHESS_MEMBER(title),
			_LICHESS_MEMBER(rating),
			_LICHESS_MEMBER(provisional)
		);
	};

	template <>
	struct class_named_member_table<GameFullEvent::Clock>
	{
		using type = GameFullEvent::Clock;
		constexpr static auto value = std::make_tuple
		(		
			_LICHESS_MEMBER(initial),
			_LICHESS_MEMBER(increment)
		);
	};

	template <>
	struct class_named_member_table<GameFullEvent>
	{
		using type = GameFullEvent;
		constexpr static auto value = std::make_tuple
		(		
			_LICHESS_MEMBER(id),
			_LICHESS_MEMBER(clock),
			_LICHESS_MEMBER(speed),
			_LICHESS_MEMBER(rated),
			_LICHESS_MEMBER(createdAt),
			_LICHESS_MEMBER(white),
			_LICHESS_MEMBER(black),
			_LICHESS_MEMBER(initialFen),
			_LICHESS_MEMBER(state)
		);
	};

	template <>
	struct class_named_member_table<Move>
	{
		using type = Move;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(ok)
		);
	};

	template <>
	struct class_named_member_table<Resign>
	{
		using type = Resign;
		constexpr static auto value = std::make_tuple
		(
			_LICHESS_MEMBER(ok)
		);
	};






	template <typename T>
	requires requires(const json& j, T& v)
	{
		v = j;
	}
	inline std::optional<T> parse_json(const httplib::Response& _response)
	{
		json _json{};
		try
		{
			_json = nlohmann::json::parse(_response.body);
		}
		catch (const json::exception& _exc)
		{
			std::cout << "[ERROR] Failed to parse json from http response\n\t" <<
				_exc.what() << '\n';
			return std::nullopt;
		};

		auto o = T();
		try
		{
			o = _json;
		}
		catch (const json::exception& _exc)
		{
			std::cout << "[ERROR] Failed to parse json object into C++\n\t" <<
				_exc.what() << '\n';
			return std::nullopt;
		};
		
		return o;
	};





	Result<AccountInfo> Client::get_account_info()
	{
		auto _result = this->client_.Get("/api/account");
		if (!_result)
		{
			return std::nullopt;
		};
		if (_result->status != 200)
		{
			return std::nullopt;
		};

		return parse_json<AccountInfo>(*_result);
	};

	Result<OngoingGames> Client::get_ongoing_games()
	{
		auto _result = this->client_.Get("/api/account/playing");
		if (!_result)
		{
			return std::nullopt;
		};
		if (_result->status != 200)
		{
			return std::nullopt;
		};

		return parse_json<OngoingGames>(*_result);
	};

	Result<Challenges> Client::get_challenges()
	{
		auto _result = this->client_.Get("/api/challenge");
		if (!_result)
		{
			return std::nullopt;
		};
		if (_result->status != 200)
		{
			return std::nullopt;
		};

		return parse_json<Challenges>(*_result);
	};

	Result<ChallengeAI> Client::challenge_ai(ChallengeAIParams _params)
	{
		auto _httpParams = httplib::Params{};
		const auto _proc = impl::ToParamMemberProc{};
		_proc.handle_members(_httpParams, &_params,
			class_named_member_table_v<ChallengeAIParams>);

		auto _result = this->client_.Post("/api/challenge/ai", _httpParams);
		if (!_result)
		{
			return std::nullopt;
		};

		if (_result->status == 201)
		{
			// Ok, no response though
			return std::nullopt;
		}
		else if (_result->status != 200)
		{
			// Error occured
			return std::nullopt;
		};

		return parse_json<ChallengeAI>(*_result);
	};

	Result<AcceptChallenge> Client::accept_challenge(AcceptChallengeParams _params)
	{
		const auto _endpoint = "/api/challenge/" + _params.challengeId + "/accept";

		auto _result = this->client_.Post(_endpoint.c_str());
		if (!_result)
		{
			return std::nullopt;
		};

		if (_result->status != 200)
		{
			// Error occured
			return std::nullopt;
		};

		return parse_json<AcceptChallenge>(*_result);
	};

	Result<Move> Client::bot_move(MoveParams _params)
	{
		const auto _endpoint = "/api/bot/game/" + _params.gameID + "/move/" + _params.move;
		auto _httpParams = httplib::Params();
		if (_params.offeringDraw)
		{
			_httpParams.insert({
				"offeringDraw", impl::tostr(*_params.offeringDraw)
				});
		};

		auto _result = this->client_.Post(_endpoint.c_str(), _httpParams);
		if (!_result)
		{
			return std::nullopt;
		};

		if (_result->status != 200)
		{
			// Error occured
			return std::nullopt;
		};

		return parse_json<Move>(*_result);
	};

	Result<Resign> Client::bot_resign(ResignParams _params)
	{
		const auto _endpoint = "/api/bot/game/" + _params.gameID + "/resign";

		auto _result = this->client_.Post(_endpoint.c_str());
		if (!_result)
		{
			return std::nullopt;
		};

		if (_result->status != 200)
		{
			// Error occured
			return std::nullopt;
		};

		return parse_json<Resign>(*_result);
	};

	Client::Client(const char* _authToken) :
		client_("https://lichess.org")
	{
		this->client_.set_bearer_token_auth(_authToken);
		//this->client_.set_logger([](const httplib::Request& _req, const httplib::Response& _res)
		//	{
		//		if (_res.has_header("Content-Type") && _res.get_header_value("Content-Type") == "application/json")
		//		{
		//			if (!_res.body.empty())
		//			{
		//				std::cout << "[DEBUG] " << json::parse(_res.body).dump(1, '\t') << '\n';
		//			};
		//		};
		//	});
	};
};

namespace lichess
{
	bool StreamClient::ndjson_content_reciever(const char* _data, size_t _len)
	{
		const auto _result = json::parse(std::string_view{ _data, _len }, nullptr, false, false);
		if (!_result.is_discarded())
		{
			return true;
		}
		else
		{
			return false;
		};
	};
};


namespace lichess
{
	void AccountEventProcessor::push(const GameStartEvent& _event) const
	{
		const auto lck = std::unique_lock(this->mtx_);
		if (auto& _cb = this->game_start_callback_; _cb)
		{
			_cb(_event);
		};
	};
	void AccountEventProcessor::push(const GameFinishEvent& _event) const
	{
		const auto lck = std::unique_lock(this->mtx_);
		if (auto& _cb = this->game_finish_callback_; _cb)
		{
			_cb(_event);
		};
	};
	void AccountEventProcessor::push(const ChallengeEvent& _event) const
	{
		const auto lck = std::unique_lock(this->mtx_);
		if (auto& _cb = this->challenge_callback_; _cb)
		{
			_cb(_event);
		};
	};

	void AccountEventProcessor::process(const json& _json)
	{
		try
		{
			if (_json.contains("type"))
			{
				const std::string _type = _json.at("type");
				if (_type == "gameStart")
				{
					GameStartEvent _event;
					_event = _json.at("game");
					this->push(_event);
				}
				else if (_type == "gameFinish")
				{
					GameFinishEvent _event;
					_event = _json.at("game");
					this->push(_event);
				}
				else if (_type == "challenge")
				{
					Challenge _event;
					_event = _json.at("challenge");
					this->push(_event);
				}
				else
				{
					// Unhandled case
				};
			};
		}
		catch (const json::exception& _exc)
		{
			std::cout << "[ERROR] Json parsing exception :\n" << _exc.what() << '\n';
		};
	};
};

namespace lichess
{
	void GameEventProcessor::push(const GameFullEvent& _event) const
	{
		const auto lck = std::unique_lock(this->mtx_);	
		if (auto& _cb = this->game_full_callback_; _cb)
		{
			_cb(_event);
		};
	};
	void GameEventProcessor::push(const GameStateEvent& _event) const
	{
		const auto lck = std::unique_lock(this->mtx_);	
		if (auto& _cb = this->game_state_callback_; _cb)
		{
			_cb(_event);
		};
	};
	
	void GameEventProcessor::process(const json& _json)
	{
		try
		{
			if (_json.contains("type"))
			{
				const std::string _type = _json.at("type");
				if (_type == "gameFull")
				{
					GameFullEvent _event;
					_event = _json;
					this->push(_event);
				}
				else if (_type == "gameState")
				{
					GameStateEvent _event;
					_event = _json;
					this->push(_event);
				}
				else
				{
					// Unhandled case
				};
			};
		}
		catch (const json::exception& _exc)
		{
			std::cout << "[ERROR] Json parsing exception :\n" << _exc.what() << '\n';
			std::cout << "  raw json = \n" << _json.dump(1, '\t') << '\n';
		};
	};
};
