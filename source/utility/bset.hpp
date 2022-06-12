#pragma once

/** @file */

#include <vector>
#include <cstdint>
#include <algorithm>
#include <unordered_set>

namespace sch
{

	struct binary_set
	{
	public:
		using value_type = uint32_t;
		using size_type = size_t;

		auto begin()
		{
			return this->data_.begin();
		};
		auto begin() const
		{
			return this->data_.begin();
		};
		auto end()
		{
			return this->data_.end();
		};
		auto end() const
		{
			return this->data_.end();
		};

		void clear() noexcept
		{
			this->data_.clear();
		};
		size_type size() const
		{
			return this->data_.size();
		};

		void insert(value_type _value)
		{
			this->data_.insert(_value);
		};
		void insert(auto it, value_type _value)
		{
			this->data_.insert(it, _value);
		};

		auto find(value_type _value)
		{
			return this->data_.find(_value);
		};
		bool contains(value_type _value) const
		{
			return this->data_.contains(_value);
		};
		


		binary_set() = default;

	private:
		std::unordered_set<value_type> data_;
	};


};