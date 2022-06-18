#pragma once

/** @file */

#include "board_base.hpp"

#include "piece.hpp"
#include "rating.hpp"
#include "bitboard.hpp"
#include "position.hpp"

#include "utility/number.hpp"
#include "utility/utility.hpp"

#include <jclib/type.h>
#include <jclib/type_traits.h>

#include <iosfwd>
#include <string>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>
#include <compare>
#include <ranges>
#include <algorithm>
#include <optional>
#include <array>
#include <span>
#include <random>

namespace chess
{
	enum class OutcomeType
	{
		none,
		draw,
		mate,
	};

	/**
	 * @brief Fufilled by types that can be used to track additional board info.
	*/
	template <typename T>
	concept cx_additional_board_info = requires(T& v, const T& cv, const Board& _board, Move _move)
	{
		v.clear();
		v.sync(_board);
		v.move(_board, _move);
	};

	/**
	 * @brief Provides additional tracking for where pieces are attacking on a chess board.
	*/
	class BoardPieceAttackData
	{
	public:

		/**
		 * @brief Resets the attack data to an initial state.
		*/
		void clear();

		/**
		 * @brief Syncs the attack data with a new board.
		 * @param _board Board to sync with.
		*/
		void sync(const Board& _board);

		/**
		 * @brief Updates the tracked data to reflect a played move.
		 * 
		 * Move legality is not checked.
		 * 
		 * @param _previousBoard The previous board state.
		 * @param _move Move to play.
		*/
		void move(const Board& _previousBoard, Move _move);

		const BitBoard& get_black_direct_attacking() const
		{
			return this->battack_;
		};
		const BitBoard& get_white_direct_attacking() const
		{
			return this->wattack_;
		};
		
		const BitBoard& get_direct_attacking(Color _player) const
		{
			return (_player == Color::white)? this->get_white_direct_attacking() : this->get_black_direct_attacking();
		};


		BoardPieceAttackData() = default;

	private:

		/**
		 * @brief Where the white pieces are attacking.
		*/
		BitBoard wattack_;

		/**
		 * @brief Where the black pieces are attacking.
		*/
		BitBoard battack_;

	};

	namespace impl
	{
		/**
		 * @brief Helper type for holding onto additional board data.
		*/
		template <cx_additional_board_info... Ts>
		class BoardExtrasImpl
		{
		public:

			/**
			 * @brief Resets the extra data to an initial state.
			*/
			void clear()
			{
				(std::get<Ts>(this->extras_).clear(), ...);
			};

			/**
			 * @brief Syncs the extra data with a new board.
			 * @param _board Board to sync with.
			*/
			void sync(const Board& _board)
			{
				(std::get<Ts>(this->extras_).sync(_board), ...);
			};

			/**
			 * @brief Updates the extra data to reflect a played move.
			 *
			 * Move legality is not checked.
			 *
			 * @param _previousBoard The previous board state.
			 * @param _move Move to play.
			*/
			void move(const Board& _previousBoard, Move _move)
			{
				(std::get<Ts>(this->extras_).move(_previousBoard, _move), ...);
			};

			template <typename T>
			auto& get() { return std::get<T>(this->extras_); };
			template <typename T>
			const auto& get() const { return std::get<T>(this->extras_); };

			BoardExtrasImpl() = default;

		private:
			std::tuple<Ts...> extras_{};
		};

		template <>
		class BoardExtrasImpl<>
		{
		public:

			/**
			 * @brief Resets the extra data to an initial state.
			*/
			void clear()
			{
			};

			/**
			 * @brief Syncs the extra data with a new board.
			 * @param _board Board to sync with.
			*/
			void sync(const Board& _board)
			{
			};

			/**
			 * @brief Updates the extra data to reflect a played move.
			 *
			 * Move legality is not checked.
			 *
			 * @param _previousBoard The previous board state.
			 * @param _move Move to play.
			*/
			void move(const Board& _previousBoard, Move _move)
			{
			};

			BoardExtrasImpl() = default;

		private:
		};
	};

	/**
	 * @brief Additional board data storage / tracking.
	*/
	using BoardExtras = impl::BoardExtrasImpl
	<
		
	>;

	std::ostream& operator<<(std::ostream& _ostr, const Board& _value);

}