#pragma once

/** @file */

#include "utility.hpp"

#include <memory>
#include <algorithm>

namespace sch
{

	/**
	 * @brief Holds a set of responses within a move tree.
	*/
	template <typename T>
	class NullTerminatedArena
	{
	public:

		using value_type = T;

		using pointer = value_type*;
		using reference = value_type&;
		using const_pointer = const value_type*;
		using const_reference = const value_type&;

		using size_type = uint8_t;

		pointer data() noexcept
		{
			return this->data_;
		};
		const_pointer data() const noexcept
		{
			return this->data_;
		};

		bool empty() const
		{
			return !this->data_ || *this->data_;
		};

		void clear() noexcept
		{
			if (auto& p = this->data_; p)
			{
				delete[] p;
				p = nullptr;
			};
		};

	private:

		void raw_set_mem(size_type s)
		{
			SCREEPFISH_ASSERT(!this->data_);
			this->data_ = new value_type[s];
			return *this;
		};

	public:

		using iterator = pointer;
		using const_iterator = const_pointer;

		iterator begin()
		{
			return this->data();
		};
		const_iterator begin() const
		{
			return this->data();
		};
		const_iterator cbegin() const
		{
			return this->data();
		};

		iterator end()
		{
			auto p = this->begin();
			for (; p && !*p; ++p) {};
			return p;
		};
		const_iterator end() const
		{
			auto p = this->begin();
			for (; p && !*p; ++p) {};
			return p;
		};
		const_iterator cend() const
		{
			auto p = this->begin();
			for (; p && !*p; ++p) {};
			return p;
		};

		size_type size() const
		{
			return this->end() - this->begin();
		};
		void resize(size_type _size)
		{
			if (_size == 0)
			{
				this->clear();
			}
			else
			{
				auto nd = value_type::create(_size);
				if (!this->empty())
				{
					std::copy(this->begin(),
						std::min(this->end(), this->begin() + _size),
						nd);
				};
				this->clear();
				this->data_ = nd;
			};
		};

		reference front()
		{
			SCREEPFISH_ASSERT(!this->empty());
			return *this->data();
		};
		const_reference front() const
		{
			SCREEPFISH_ASSERT(!this->empty());
			return *this->data();
		};

		reference back()
		{
			SCREEPFISH_ASSERT(!this->empty());
			return *(this->data() + this->size() - 1);
		};
		const_reference back() const
		{
			SCREEPFISH_ASSERT(!this->empty());
			return *(this->data() + this->size() - 1);
		};

		NullTerminatedArena() = default;
		NullTerminatedArena(const NullTerminatedArena& other) = delete;

		NullTerminatedArena& operator=(const NullTerminatedArena& other)
		{
			this->clear();
			this->resize(other.size());
			std::copy(other.begin(), other.end(), this->begin());
			return *this;
		};

		NullTerminatedArena(NullTerminatedArena&& other) noexcept :
			data_(std::exchange(other.data_, nullptr))
		{};
		NullTerminatedArena& operator=(NullTerminatedArena&& other) noexcept
		{
			this->clear();
			this->data_ = std::exchange(other.data_, nullptr);
			return *this;
		};

		~NullTerminatedArena()
		{
			this->clear();
		};

	private:

		pointer data_{};
	};


	/**
	 * @brief Holds a set of responses within a move tree.
	*/
	template <typename T>
	class NullTerminatedArenaInterface
	{
	private:

		// expected 
		// T* create(size_type n);

		// expected 
		// void destroy(T* p);

		// expected
		// bool is_null(const T& v);

		T& as_crtp() noexcept { return static_cast<T&>(*this); };
		const T& as_crtp() const noexcept { return static_cast<const T&>(*this); };

	public:

		using value_type = T;

		using pointer = value_type*;
		using reference = value_type&;
		using const_pointer = const value_type*;
		using const_reference = const value_type&;

		using size_type = uint8_t;

		pointer data() noexcept
		{
			return this->data_;
		};
		const_pointer data() const noexcept
		{
			return this->data_;
		};

		bool empty() const
		{
			return !this->data_ || this->as_crtp().is_null(*this->data_);
		};

		void clear() noexcept
		{
			if (auto& p = this->data_; p)
			{
				this->as_crtp().destroy(p);
				p = nullptr;
			};
		};

	private:

		void raw_set_mem(size_type s)
		{
			SCREEPFISH_ASSERT(!this->data_);
			this->data_ = this->as_crtp().create(s);
			return *this;
		};

	public:

		using iterator = pointer;
		using const_iterator = const_pointer;

		iterator begin()
		{
			return this->data();
		};
		const_iterator begin() const
		{
			return this->data();
		};
		const_iterator cbegin() const
		{
			return this->data();
		};

		iterator end()
		{
			auto p = this->begin();
			for (; p && !this->as_crtp().is_null(*p); ++p) {};
			return p;
		};
		const_iterator end() const
		{
			auto p = this->begin();
			for (; p && !this->as_crtp().is_null(*p); ++p) {};
			return p;
		};
		const_iterator cend() const
		{
			auto p = this->begin();
			for (; p && !this->as_crtp().is_null(*p); ++p) {};
			return p;
		};

		size_type size() const
		{
			return this->end() - this->begin();
		};
		void resize(size_type _size)
		{
			if (_size == 0)
			{
				this->clear();
			}
			else
			{
				auto nd = value_type::create(_size);
				if (!this->empty())
				{
					std::copy(this->begin(),
						std::min(this->end(), this->begin() + _size),
						nd);
				};
				this->clear();
				this->data_ = nd;
			};
		};

		reference front()
		{
			SCREEPFISH_ASSERT(!this->empty());
			return *this->data();
		};
		const_reference front() const
		{
			SCREEPFISH_ASSERT(!this->empty());
			return *this->data();
		};

		reference back()
		{
			SCREEPFISH_ASSERT(!this->empty());
			return *(this->data() + this->size() - 1);
		};
		const_reference back() const
		{
			SCREEPFISH_ASSERT(!this->empty());
			return *(this->data() + this->size() - 1);
		};

		NullTerminatedArenaInterface() = default;
		NullTerminatedArenaInterface(const NullTerminatedArenaInterface& other) = delete;
		NullTerminatedArenaInterface& operator=(const NullTerminatedArenaInterface& other)
		{
			this->clear();
			this->resize(other.size());
			std::copy(other.begin(), other.end(), this->begin());
			return *this;
		};

		NullTerminatedArenaInterface(NullTerminatedArenaInterface&& other) noexcept :
			data_(std::exchange(other.data_, nullptr))
		{};
		NullTerminatedArenaInterface& operator=(NullTerminatedArenaInterface&& other) noexcept
		{
			this->clear();
			this->data_ = std::exchange(other.data_, nullptr);
			return *this;
		};

		~NullTerminatedArenaInterface()
		{
			this->clear();
		};

	private:

		pointer data_{};
	};


}