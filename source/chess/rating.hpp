#pragma once

/** @file */

#include "piece.hpp"

#include <compare>

namespace chess
{
	/**
	 * @brief Holds a raw rating value.
	*/
	using Rating = float;

	consteval Rating operator""_rt(long double v)
	{
		return static_cast<Rating>(v);
	};
	consteval Rating operator""_rt(unsigned long long v)
	{
		return static_cast<Rating>(v);
	};




	/**
	 * @brief Holds an absolute rating, positive is better for white, negative is better for black.
	*/
	struct AbsoluteRating
	{
	public:

		using rep = Rating;

		/**
		 * @brief Gets the raw rating value.
		 * @return Raw rating.
		*/
		constexpr Rating raw() const noexcept
		{
			return this->value_;
		};

		/**
		 * @brief Gets the raw rating value for a given player.
		 * @param _player The player to get the rating for.
		 * @return Rating for the given player.
		*/
		constexpr Rating player(Color _player) const noexcept
		{
			return (_player == Color::white)? this->raw() : -this->raw();
		};

		constexpr auto operator<=>(const AbsoluteRating& rhs) const noexcept = default;
		
		constexpr AbsoluteRating& operator+=(Rating rhs)
		{
			this->value_ += rhs;
			return *this;
		};
		constexpr AbsoluteRating& operator-=(Rating rhs)
		{
			this->value_ -= rhs;
			return *this;
		};



		/**
		 * @brief Gets the raw rating value for a white.
		 * @return Rating for white.
		*/
		constexpr Rating white() const noexcept
		{
			return this->raw();
		};

		/**
		 * @brief Gets the raw rating value for a black.
		 * @return Rating for black.
		*/
		constexpr Rating black() const noexcept
		{
			return -this->raw();
		};




		constexpr AbsoluteRating() noexcept = default;
		constexpr explicit AbsoluteRating(rep _value) noexcept :
			value_(_value)
		{};

		constexpr explicit AbsoluteRating(rep _value, Color _player) noexcept :
			value_((_player == Color::white)? _value : -_value)
		{};

	private:
		rep value_;
	};



	consteval AbsoluteRating operator""_art(long double v)
	{
		return AbsoluteRating(static_cast<AbsoluteRating::rep>(v));
	};
	consteval AbsoluteRating operator""_art(unsigned long long v)
	{
		return AbsoluteRating(static_cast<AbsoluteRating::rep>(v));
	};

};
