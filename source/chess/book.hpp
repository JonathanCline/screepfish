#pragma once

/** @file */

#include "move.hpp"
#include "utility/arena.hpp"

#include <jclib/algorithm.h>
#include <jclib/functional.h>

#include <vector>

namespace chess
{
	struct Book
	{
	public:

		struct Branch;
		using node_type = Branch;

		struct BranchBlockAllocator
		{
			using value_type = node_type;

			value_type* allocate(size_t n) const;
			void deallocate(value_type* p) const;
		};

		struct Branch
		{
		public:

			using key_type = Move;
			using element_type = Branch;

			using value_type = std::pair<key_type, element_type>;

		private:

			using container_type = std::vector<value_type>;

		public:
			using iterator = typename container_type::iterator;
			using const_iterator = typename container_type::const_iterator;

			iterator begin() noexcept { return this->responses_.begin(); };
			const_iterator begin() const noexcept { return this->responses_.cbegin(); };
			const_iterator cbegin() const noexcept { return this->responses_.cbegin(); };

			iterator end() noexcept { return this->responses_.end(); };
			const_iterator end() const noexcept { return this->responses_.cend(); };
			const_iterator cend() const noexcept { return this->responses_.cend(); };



			/**
			 * @brief The move to play.
			 * @return Chess move.
			*/
			key_type move() const
			{
				return this->move_;
			};
			
			bool empty() const
			{
				return this->responses_.empty();
			};

		private:

			/**
			 * @brief Finds the best registered response to a move.
			 * @param _branch Branch reference to look at.
			 * @param _move Move to find a response to.
			 * @return Iterator to best response, or .end() if not found.
			*/
			static auto find_response_impl(auto& _branch, const key_type& _move)
			{
				return jc::find_if(_branch.responses_, [_move](const value_type& _node) -> bool
					{
						return _node.first == _move;
					});
			};

		public:

			/**
			 * @brief Finds the best registered response to a move.
			 * @param _move Move to find a response to.
			 * @return Iterator to best response, or .end() if not found.
			*/
			iterator find_response(const key_type& _move)
			{
				return find_response_impl(*this, _move);
			};

			/**
			 * @brief Finds the best registered response to a move.
			 * @param _move Move to find a response to.
			 * @return Iterator to best response, or .end() if not found.
			*/
			const_iterator find_response(const key_type& _move) const
			{
				return find_response_impl(*this, _move);
			};

			/**
			 * @brief Checks if this branch has a response to a move.
			 * @param _move Move to check for a response to.
			 * @return True if contained, false otherwise.
			*/
			bool has_response(const key_type& _move) const
			{
				return this->find_response(_move) != this->end();
			};

			/**
			 * @brief Clears the branch.
			*/
			void clear() noexcept
			{
				this->responses_.clear();
				this->move_ = key_type(jc::null);
			};

			template <jc::cx_iterator_to<std::pair<Move, Move>> IterT> 
			void assign(IterT _begin, IterT _end)
			{
				auto& _storage = this->responses_;
				_storage.resize(std::distance(_begin, _end));
				std::transform(_begin, _end, _storage.begin(), [](const std::pair<Move, Move>& p) -> value_type
					{
						return value_type{ p.first, Branch(p.second) };
					});
			};

			explicit Branch(key_type _move) noexcept :
				move_(_move)
			{};

			Branch(jc::null_t) noexcept :
				Branch(key_type(jc::null))
			{}; 
			Branch() noexcept :
				Branch(jc::null)
			{};

		private:

			/**
			 * @brief The "best" move to respond with.
			*/
			key_type move_;

			/**
			 * @brief Branches for likely opponent responses.
			*/
			container_type responses_;
		};

		/**
		 * @brief Gets the root node for the tree.
		 * @return Root node reference.
		*/
		node_type& root() noexcept
		{
			return this->root_;
		};

		/**
		 * @brief Gets the root node for the tree.
		 * @return Root node reference.
		*/
		const node_type& root() const noexcept
		{
			return this->root_;
		};

		/**
		 * @brief Clears the tree, leaving only a null root node.
		*/
		void clear() noexcept
		{
			this->root_.clear();
		};

		Book() = default;

	private:

		/**
		 * @brief Opening book root node, its move will be null.
		*/
		node_type root_;

	};
};
