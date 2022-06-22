#pragma once

/** @file */

#include "utility.hpp"
#include "utility/block_allocator.hpp"

#include <memory>
#include <algorithm>

namespace sch
{

	/**
	 * @brief Holds a set of responses within a move tree.
	*/
	template <typename T, sch::cx_block_allocator AllocatorT = sch::block_allocator<T>>
	class NullTerminatedArena
	{
	public:

		using value_type = T;
		using allocator_type = AllocatorT;

		using pointer = value_type*;
		using reference = value_type&;
		using const_pointer = const value_type*;
		using const_reference = const value_type&;


		using size_type = size_t;

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
			return !this->data_ || !*this->data_;
		};

		auto allocator() const { return allocator_type{}; };

		void clear() noexcept
		{
			if (auto& p = this->data_; p)
			{
				this->allocator().deallocate(p);
				p = nullptr;
			};
		};

	private:

		pointer allocate_block(size_type n)
		{
			auto _alloc = this->allocator();
			auto _mem = _alloc.allocate(n + 1);
			SCREEPFISH_ASSERT(!_mem[n]);
			return _mem;
		};
		void raw_set_mem(size_type s)
		{
			SCREEPFISH_ASSERT(!this->data_);
			this->data_ = this->allocate_block(s);
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
			for (; p && *p; ++p) {};
			return p;
		};
		const_iterator end() const
		{
			auto p = this->begin();
			for (; p && *p; ++p) {};
			return p;
		};
		const_iterator cend() const
		{
			auto p = this->begin();
			for (; p && *p; ++p) {};
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
				auto nd = this->allocate_block(_size);
				if (!this->empty())
				{
					const_pointer sp = this->begin();
					const const_pointer spe = this->end();

					pointer dp = nd;
					const const_pointer dpe = dp + _size;

					for (; sp != spe && dp != dpe; ++sp, ++dp)
					{
						*dp = *sp;
					};
				};
				this->clear();
				this->data_ = nd;
			};
		};
		void resize(size_type _size, const_reference _fill)
		{
			if (_size == 0)
			{
				this->clear();
			}
			else
			{
				this->clear();
				this->raw_set_mem(_size);
				std::fill_n(this->data_, _size, _fill);
			};
		};

		void soft_resize(size_type _size)
		{
			if (this->data_)
			{
				this->data_[_size] = value_type{};
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


		template <typename IterT>
		void assign(IterT _begin, IterT _end)
		{
			this->clear();
			if (_begin != _end)
			{
				this->raw_set_mem(std::distance(_begin, _end));
				std::copy(_begin, _end, this->data());
			};
		};




		NullTerminatedArena() :
			data_(nullptr)
		{};
		NullTerminatedArena(const NullTerminatedArena& other) :
			NullTerminatedArena()
		{
			if (other.data())
			{
				this->raw_set_mem(other.size());
				std::copy(other.begin(), other.end(), this->begin());
			};
		};
		NullTerminatedArena& operator=(const NullTerminatedArena& other)
		{
			this->clear();
			if (other.data())
			{
				this->raw_set_mem(other.size());
				std::copy(other.begin(), other.end(), this->begin());
			};
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