#pragma once

/** @file */

#include "board.hpp"

namespace chess
{
	struct ZobristHashTable
	{
		using type = uint32_t;

		std::array<std::array<type, 12>, 64> table{};
		type black_to_move{};
	};

	inline auto zobrist_hash_subindex(PieceType p, Color c)
	{
		size_t _subindex = 0;
		if (c == Color::white) { _subindex = 1; };
		_subindex |= static_cast<size_t>(jc::to_underlying(p) - 1) << 1;
		return _subindex;
	};

	template <typename T = uint64_t>
	constexpr T pseudorand(T _value,
		const T _largeA = 125361361361603, const T _largeB = 995995959582, 
		const T _mod = 10000)
	{
		return (static_cast<T>(_largeA * _value) + _largeB) % _mod;
	};

	inline auto zobrist_hash_index(File f, Rank r)
	{
		ZobristHashTable::type _hash = 0;
		_hash |= ((jc::to_underlying(f) << 3) | jc::to_underlying(r));
		return _hash;
	};

	inline auto zobrist_hash_table()
	{
		static thread_local std::random_device rnd_dv{};
		static thread_local std::mt19937 mt{rnd_dv()};

		auto _table = ZobristHashTable{};
		for (auto& f : files_v)
		{
			for (auto& r : ranks_v)
			{
				const auto i = zobrist_hash_index(f, r);
				for (auto& p : piece_types_v)
				{
					if (p == PieceType::none)
						continue;
					
					for (auto& c : colors_v)
					{
						const auto j = zobrist_hash_subindex(p, c);
						const auto _pseudoRand = static_cast<ZobristHashTable::type>(mt());
						const auto _hash = _pseudoRand;
						_table.table[i][j] = _hash;
					};
				};
			};
		};

		_table.black_to_move = pseudorand(static_cast<ZobristHashTable::type>(mt()));
		return _table;
	};

	static const inline auto zobrist_hash_lookup_table_v =
		zobrist_hash_table();

	/**
	 * @brief Calculates a zobrist hash for a board.
	 * @param _board Chess board.
	 * @param _blackToMove Whether or not it is blacks turn to move.
	 * @return Zobrist hash.
	*/
	inline auto hash(const Board& _board, bool _blackToMove)
	{
		ZobristHashTable::type _hash = 0;
		if (_blackToMove)
		{
			_hash ^= zobrist_hash_lookup_table_v.black_to_move;
		};

		for (auto& f : files_v)
		{
			for (auto& r : ranks_v)
			{
				const auto p = _board.get(f, r);
				if (p)
				{
					const auto i = zobrist_hash_index(f, r);
					const auto j = zobrist_hash_subindex(p.type(), p.color());
					_hash ^= zobrist_hash_lookup_table_v.table[i][j];
				};
			};
		};

		return _hash;
	};

	/**
	 * @brief Calculates a zobrist hash for a board.
	 * @param _board Chess board.
	 * @return Zobrist hash.
	*/
	inline auto hash(const Board& _board)
	{
		const auto _blackToMove = (_board.get_toplay() == Color::black);
		return hash(_board, _blackToMove);
	};





	/**
	 * @brief Resets a board to the standard chess starting positions.
	 * @param _board The board to reset.
	 * @return The given board, now reset.
	*/
	Board& reset_board(Board& _board);



	class IGame
	{
	public:

		const Board& board() const
		{
			return this->board_;
		};

		Board board_;
		
		IGame() = default;
	protected:
		~IGame() = default;
	};

	struct Response
	{
		std::optional<Move> move;

		Response() = default;
	};

	class IChessEngine
	{
	public:

		virtual void set_board(const chess::Board& _board) = 0;
		virtual Response get_move() = 0;
		virtual void start(chess::Board _initialBoard, chess::Color _color) = 0;
		virtual void stop() = 0;

		IChessEngine() = default;
		virtual ~IChessEngine() = default;
	};
};
