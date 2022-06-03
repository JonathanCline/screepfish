#include "screepfish.hpp"

#include "tests/tests.hpp"

#include "env.hpp"

#include "engine/engine.hpp"

#include "chess/chess.hpp"
#include "chess/fen.hpp"

#include "lichess/lichess.hpp"

#include "utility/perf.hpp"

#include "terminal/terminal.hpp"

#include <jclib/functor.h>

#include <array>
#include <vector>
#include <utility>
#include <iostream>
#include <filesystem>

namespace sch
{


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

			this->engine_.set_board(_board);

			if (*this->my_turn_)
			{
				const auto _response = this->engine_.get_move();
				bool _passed = false;
				if (_response.move)
				{
					auto _params = lichess::MoveParams();
					_params.gameID = this->game_id_;
					std::stringstream sstr{};
					sstr << *_response.move;
					_params.move = sstr.str();
					_passed = this->client_.bot_move(_params).has_value();
				};
				if (!_passed)
				{
					auto _resignParams = lichess::ResignParams();
					_resignParams.gameID = this->game_id_;

					if (_response.move)
					{
						std::cout << "[ERROR] Failed to submit move : " << *_response.move << '\n';
						std::cout << _board << '\n';
						std::cout << chess::get_fen(_board) << '\n';
					};

					this->client_.bot_resign(_resignParams);
				};
			};
		};

		void on_game_full(const lichess::GameFullEvent& _event)
		{
			std::cout << "[Debug] on_game_full" << std::endl;



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

			// Lichess likes to give startpos as an initial fen string, switch that to actual fen if recieved
			std::string_view _fen = _event.initialFen;
			if (_fen == "startpos")
			{
				_fen = chess::standard_start_pos_fen_v;
			};

			// Parse fen into board state
			auto _board = chess::parse_fen(_fen);
			if (!_board)
			{
				std::cout << _event.initialFen << '\n';
				abort();
			};

			// Tell the engine which color it is playing as
			this->engine_.start(*_board, *this->my_color_);

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

		std::string_view game_id() const& noexcept
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
			this->engine_.stop();
			this->keep_open_ = false;
		};

		void enable_logging(std::string _executableDirectory)
		{
			this->engine_.set_logging_dir(_executableDirectory + "/" + std::string(this->game_id()));
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
				//sch::open_browser(_url.c_str());
			};

			std::cout << "Started game " << _event.id << '\n';

			// Create the new stream
			this->game_streams_.emplace_back(this->env_.token.c_str(), _event.id, this->account_info_.id);
			this->game_streams_.back().enable_logging(this->env_.executable_root_path + "/logs");
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

			std::cout << "Logged in as user = " << this->account_info_.username << std::endl;

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
					//this->game_streams_.emplace_back(this->env_.token.c_str(), v.gameId, this->account_info_.id);
				};

				if (_games->nowPlaying.empty())
				{
					using namespace std::chrono_literals;

					std::this_thread::sleep_for(std::chrono::seconds(1));

					auto _params = lichess::ChallengeAIParams{};
					_params.level = 4;
					_params.days.reset();
					_params.clock.emplace();
					//_params.clock->set_initial(1min);
					//_params.clock->set_increment(3s);
					auto _result = this->account_client_.challenge_ai(_params);
					if (!_result)
					{
						std::cout << "[Error] Failed to challenge the AI - " <<
							_result.alternate().error_ << " - " <<
							_result.alternate().status_ << '\n';
					}
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
							std::cout << "[Error] Failed to accept challenge with ID " <<
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

};

namespace sch
{

	bool run_tests_main()
	{
		bool _runOnFinish = true;

		{
			bool _failed = false;
			const auto _results = run_tests();
			for (auto& v : _results)
			{
				if (!v)
				{
					std::cout << "TEST FAILED " <<
						v.name() <<
						"(" << v.result() << ") : " <<
						v.description() << std::endl;
					_failed = true;
				};
			};

			if (_failed && !_runOnFinish)
			{
				return _runOnFinish;
			};
		}


		using namespace chess;

		{
			const auto _fen = "r3k1nr/pppn1ppp/4b3/4q3/Pb5P/8/3PP1P1/RNBQKBNR w KQkq - 0 8";
			auto _board = *parse_fen(_fen);
			_board.move(Move((File::d, Rank::r2), (File::d, Rank::r4)));
			if (!is_check(_board, Color::white))
			{
				std::cout << _board << std::endl;
				abort();
			};
		};

		{
			const auto _fen = "1rb1kbnr/ppNppppp/2n5/6NQ/4P3/3P4/PPP2PPq/R3KB1R b KQk - 1 11";
			const auto _board = *parse_fen(_fen);
			if (!is_check(_board, Color::black))
			{
				abort();
			};
		};


		{
			auto _board = Board();
			reset_board(_board);
			const auto rt0 = rate_board(_board, Color::white);
			_board.move((File::a, Rank::r2), (File::a, Rank::r4));
			const auto rt1 = rate_board(_board, Color::white);

			// rt1 should be slightly higher
			if (rt0 >= rt1)
			{
				std::cout << rt1 << " should be greater than " << rt0 << '\n';
				abort();
			};
		};

		{
			auto _board = Board();
			reset_board(_board);

			size_t _bRooks = 0;
			size_t _wRooks = 0;

			for (auto& p : _board.pieces())
			{
				if (p == Piece::rook)
				{
					if (p.color() == Color::white)
					{
						++_wRooks;
					}
					else
					{
						++_bRooks;
					};
				};
			};

			if (_bRooks != 2) { abort(); };
			if (_wRooks != 2) { abort(); };
		};

		// Rook blocked
		{
			auto _board = Board();
			reset_board(_board);

			if (!chess::is_rook_blocked(_board, (File::a, Rank::r1), Color::white))
			{
				abort();
			};
			if (!chess::is_rook_blocked(_board, (File::h, Rank::r1), Color::white))
			{
				abort();
			};
			if (!chess::is_rook_blocked(_board, (File::a, Rank::r8), Color::black))
			{
				abort();
			};
			if (!chess::is_rook_blocked(_board, (File::h, Rank::r8), Color::black))
			{
				abort();
			};

			std::cout << _board << std::endl;
		};

		// Queen blocked rating
		{
			Rating rt0 = 0.0f;
			Rating rt1 = 0.0f;

			{
				auto _board = Board();
				reset_board(_board);
				rt0 = rate_board(_board, Color::white);
			};

			{
				auto _board = Board();
				reset_board(_board);
				_board.erase_piece((File::d, Rank::r2));
				_board.erase_piece((File::d, Rank::r7));
				rt1 = rate_board(_board, Color::white);
				const auto rt1b = rate_board(_board, Color::black);

				if (chess::is_queen_blocked(_board, (File::d, Rank::r1), Color::white))
				{
					abort();
				};
				if (chess::is_queen_blocked(_board, (File::d, Rank::r8), Color::black))
				{
					abort();
				};

				// rt1 should be slightly higher
				if (rt0 >= rt1)
				{
					std::cout << rt1 << " should be greater than " << rt0 << '\n';
					//abort();
				};
			};
		};

		// Mate in 1
		{
			const auto _fen = "6rn/8/8/8/K7/2k5/1q6/8 b - - 92 118";
			auto _board = *parse_fen(_fen);

			if (chess::is_checkmate(_board, Color::white))
			{
				// Shouldn't be checkmate
				abort();
			};

			auto _engine = sch::ScreepFish();
			_engine.start(_board, Color::black);
			_engine.set_board(_board);
			const auto _move = _engine.get_move();

			_board.move(*_move.move);
			if (!chess::is_checkmate(_board, Color::white))
			{
				// Should be checkmate
				abort();
			};
		};

		// Bishop causing check
		{
			auto _board = Board();
			reset_board(_board);
			_board.erase_piece((File::f, Rank::r2));
			_board.move((File::f, Rank::r8), (File::h, Rank::r4));

			if (!is_check(_board, Color::white))
			{
				// Should be check
				abort();
			};
		};

		// Rook check
		{
			auto _board = *parse_fen("4r3/2bk1p2/8/PbP5/1P5p/5P2/1R5P/1N2K2R w K - 11 39");
			if (!is_check(_board, Color::white))
			{
				// Should be check
				std::cout << _board << std::endl;
				abort();
			};

			_board.move((File::b, Rank::r2), (File::d, Rank::r2));
			if (!is_piece_attacked_by_rook(_board,
				_board.get_white_king(),
				*_board.pfind((File::e, Rank::r8))
			))
			{
				auto _data = std::array<Move, 32>{};
				auto _buffer = MoveBuffer(_data);
				get_rook_moves(_board, *_board.pfind((File::e, Rank::r8)), _buffer);
				for (auto bp = _data.data(); bp != _buffer.head(); ++bp)
				{
					std::cout << *bp << std::endl;
				};
				std::cout << _board << std::endl;
				abort();
			};

			if (!is_piece_attacked(_board, _board.get_white_king()))
			{
				// Should be check
				std::cout << _board << std::endl;
				abort();
			};

			if (!is_check(_board, Color::white))
			{
				// Should be check
				std::cout << _board << std::endl;
				abort();
			};

			auto _data = std::array<Move, 32>{};
			auto _buffer = MoveBuffer(_data);
			const auto b0 = _buffer.head();
			get_moves(_board, Color::white, _buffer);
			const auto b1 = _buffer.head();

			for (auto bp = b0; bp != b1; ++bp)
			{
				std::cout << *bp << std::endl;

				auto nb = _board;
				nb.move(*bp);
				if (is_check(nb, Color::white))
				{
					// Should NEVER be check
					std::cout << nb << std::endl;
					abort();
				};
			};
			
		};

		return _runOnFinish;
	};

	template <size_t Runs = 5>
	auto perf_test_part(auto&& _op, std::chrono::milliseconds _duration = std::chrono::seconds(1))
	{
		using namespace chess;

		std::array<size_t, Runs> _runs{};

		for (auto& r : _runs)
		{
			using namespace std::chrono_literals;
			r = sch::count_runs_within_duration(_op, _duration);
		};

		const auto _runsTotal = std::accumulate(_runs.begin(), _runs.end(), (size_t)0);
		const auto _runsAvg = _runsTotal / _runs.size();
		return _runsAvg;
		std::cout << "rt : " << _runsAvg << std::endl;
	};

	void perf_test()
	{
		using namespace chess;

		// midgame, depth 3 - unique
		{
			auto b = *parse_fen("rn2kbnr/p2b1pp1/4p3/q2P3p/p2Q4/2N2N2/1PBB1PPP/R3K2R b KQkq - 1 13");
			const auto fn = [&b]()
			{
				auto t = MoveTree();
				t.board = b;
				t.build_tree(3);
			};
			const auto r = perf_test_part(fn);
			std::cout << "midgame (d3_unique) - " << r << std::endl;
		};

		// midgame, depth 3
		{
			auto b = *parse_fen("rn2kbnr/p2b1pp1/4p3/q2P3p/p2Q4/2N2N2/1PBB1PPP/R3K2R b KQkq - 1 13");
			const auto fn = [&b]()
			{
				auto t = MoveTree();
				t.board = b;
				t.evaluate_next();
				t.evaluate_next();
				t.evaluate_next();
			};
			const auto r = perf_test_part(fn);
			std::cout << "midgame (d3) - " << r << std::endl;
		};

		// opening, depth 2
		{
			auto b = Board();
			reset_board(b);
			const auto fn = [&b]()
			{
				auto t = MoveTree();
				t.board = b;
				t.evaluate_next();
				t.evaluate_next();
			};
			const auto r = perf_test_part(fn);
			std::cout << "opening (d2) - " << r << std::endl;
		};

		// midgame, depth 2
		{
			auto b = *parse_fen("rn2kbnr/p2b1pp1/4p3/q2P3p/p2Q4/2N2N2/1PBB1PPP/R3K2R b KQkq - 1 13");
			const auto fn = [&b]()
			{
				auto t = MoveTree();
				t.board = b;
				t.evaluate_next();
				t.evaluate_next();
			};
			const auto r = perf_test_part(fn);
			std::cout << "midgame (d2) - " << r << std::endl;
		};

	};

	bool local_game(const char* _assetsDirectoryPath, bool _step)
	{
		using namespace chess;
		auto _terminal = chess::Terminal(_assetsDirectoryPath, _step);

		auto _board = Board();
		reset_board(_board);
		_terminal.set_board(_board);

		auto e0 = sch::ScreepFish();
		auto e1 = sch::ScreepFish();

		auto _move = Move();

		e0.start(_board, Color::white);
		if (const auto m = e0.get_move(); m.move) {
			_move = *m.move;
		};

		_board.move(_move);
		_terminal.set_board(_board);
		_terminal.step();

		e1.start(_board, Color::black);

		while (true)
		{
			if (_terminal.should_close())
			{
				return false;
			};

			std::cout << "b: ";
			e1.set_board(_board);
			if (auto m = e1.get_move(); m.move)
			{
				_move = *m.move;
			}
			else
			{
				std::cout << "white wins\n";
				break;
			};

			_board.move(_move);
			std::cout << get_fen(_board) << std::endl;

			_terminal.set_board(_board);
			_terminal.step();

			if (_board.get_half_move_count() >= 50)
			{
				std::cout << "50 move rule" << std::endl;
				break;
			};

			std::cout << "w: ";
			e0.set_board(_board);
			if (auto m = e0.get_move(); m.move)
			{
				_move = *m.move;
			}
			else
			{
				std::cout << "black wins\n";
				break;
			};

			_board.move(_move);
			std::cout << get_fen(_board) << std::endl;

			_terminal.set_board(_board);
			_terminal.step();

			if (_board.get_half_move_count() >= 50)
			{
				std::cout << "50 move rule" << std::endl;
				break;
			};
		};

		_terminal.wait_for_any_key();
		return !_terminal.should_close();
	};



	int local_game_main(int _nargs, const char* _vargs[])
	{
		const auto p = (std::filesystem::path(_vargs[0]).parent_path() / "assets" / "chess").generic_string();
		std::cout << p << '\n';

		bool _step = false;
		bool _one = false;
		for (int n = 1; n < _nargs; ++n)
		{
			const auto _arg = std::string_view(_vargs[n]);
			if (_arg == "--step")
			{
				_step = true;
			}
			else if (_arg == "--one")
			{
				_one = true;
			};
		};

		bool _keepAlive = !_one;
		while (_keepAlive)
		{
			_keepAlive = sch::local_game(p.c_str(), _step);
		};

		return 0;
	};

	int lichess_bot_main(int _nargs, const char* _vargs[])
	{
		// Renable when lichess lets us play the AI
		const auto _env = sch::load_env(_vargs[0]);
		auto _accountManager = sch::AccountManager(_env);
		_accountManager.start();

		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			_accountManager.update();
		};

		return 0;
	};
}
