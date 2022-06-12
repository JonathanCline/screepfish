#pragma once

/** @file */

#include "move.hpp"
#include "rating.hpp"

#include "utility/bset.hpp"

#include <set>
#include <vector>
#include <random>
#include <unordered_set>

namespace chess
{
	using BoardHashSet = sch::binary_set;

	/**
	 * @brief Holds a set of responses within a move tree.
	*/
	template <typename T>
	requires requires (const T& v, bool b)
	{
		b = v;
	}
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
			return this->data_.get();
		};
		const_pointer data() const noexcept
		{
			return this->data_.get();
		};

		bool empty() const
		{
			return !this->data_ || !this->data_[0];
		};

	private:

		pointer mem_begin()
		{
			return this->data();
		};
		const_pointer mem_begin() const
		{
			return this->data();
		};
		pointer mem_end()
		{
			return this->begin() + this->size();
		};
		const_pointer mem_end() const
		{
			return this->begin() + this->size();
		};

	public:

		using iterator = pointer;
		using const_iterator = const_pointer;

		iterator begin()
		{
			return this->mem_begin();
		};
		const_iterator begin() const
		{
			return this->mem_begin();
		};
		const_iterator cbegin() const
		{
			return this->mem_begin();
		};

		iterator end()
		{
			return std::find(this->mem_begin(), this->mem_end(), jc::null);
		};
		const_iterator end() const
		{
			return std::find(this->mem_begin(), this->mem_end(), jc::null);
		};
		const_iterator cend() const
		{
			return std::find(this->mem_begin(), this->mem_end(), jc::null);
		};

		size_type size() const
		{
			size_type n = 0;
			return std::find(this->begin(), this->end(), jc::null) - this->end();
		};
		void resize(size_type _size)
		{
			if (_size == 0)
			{
				this->clear();
			}
			else if (this->empty())
			{
				this->data_ = std::make_unique<value_type[]>(_size);
			}
			else
			{
				auto nd = std::make_unique<value_type[]>(_size);
				std::copy(this->begin(), this->end(), nd.get());
				this->data_ = std::move(nd);
			};
		};

		reference front()
		{
			SCREEPFISH_ASSERT(!this->empty());
			return this->data_[0];
		};
		const_reference front() const
		{
			SCREEPFISH_ASSERT(!this->empty());
			return this->data_[0];
		};

		reference back()
		{
			SCREEPFISH_ASSERT(!this->empty());
			return this->data_[this->size() - 1];
		};
		const_reference back() const
		{
			SCREEPFISH_ASSERT(!this->empty());
			return this->data_[this->size() - 1];
		};

		void clear() noexcept
		{
			this->data_.reset();
		};

		NullTerminatedArena() = default;

	private:

		std::unique_ptr<value_type[]> data_{};
	};



	struct MoveTreeNode
	{
	private:

		void resort_children();

	public:

		using size_type = uint8_t;

		/**
		 * @brief Gets the full rating for the position - factoring in child nodes.
		 * @return Full rating.
		*/
		Rating rating() const
		{
			return this->rating_;
		};

		void set_rating(Rating r) { this->rating_ = r; };

		/**
		 * @brief Gets the quick rating for the position.
		 * @return Quick rating.
		*/
		Rating quick_rating() const
		{
			return this->move.rating();
		};


		bool empty() const
		{
			return this->responses_count_ == 0;
		};
		void resize(size_type _size)
		{
			this->responses_ = std::make_unique<MoveTreeNode[]>(_size);
			this->responses_count_ = _size;
		};

		size_type size() const { return this->responses_count_; };

		auto begin() { return this->responses_.get(); };
		auto begin() const { return this->responses_.get(); };
		auto end() { return this->begin() + this->size(); };
		auto end() const { return this->begin() + this->size(); };

		auto& front()
		{
			return this->responses_[0];
		};
		const auto& front() const
		{
			return this->responses_[0];
		};

		void count_duplicates(Board _board, std::set<size_t>& _boards);

		void evaluate_next(const Board& _previousBoard, BoardHashSet& _hashSet, bool _followChecks = true);
		void evaluate_next(const Board& _previousBoard, bool _followChecks = true);


		size_t tree_size() const;
		size_t total_outcomes() const;

		size_t count_checks(Board _board);

		void show_best_line() const;
		std::vector<RatedMove> get_best_line() const;

		MoveTreeNode() = default;

	private:
		std::unique_ptr<MoveTreeNode[]> responses_{};
	public:
		RatedMove move{};
	private:
		Rating rating_ = 0;
		uint8_t responses_count_ = 0;

	};




	struct MoveTree
	{
		void evaluate_next();
		void evaluate_next_unique();
		void evalulate_next();

		std::optional<RatedMove> best_move(std::mt19937& _rnd);
		size_t tree_size() const;
		size_t total_outcomes() const;

		std::vector<std::vector<RatedMove>> get_top_lines(size_t _maxCount) const;

		size_t count_unique_positions();
		size_t count_checks();

		void clear_hashes() { this->hash_set_.clear(); };

		void build_tree(size_t _depth);

		auto begin() { return this->moves_.begin(); };
		auto begin() const { return this->moves_.begin(); };
		auto cbegin() const { return this->moves_.begin(); };
		auto end() { return this->moves_.end(); };
		auto end() const { return this->moves_.end(); };
		auto cend() const { return this->moves_.end(); };

		const chess::Board& initial_board() const
		{
			return this->board_;
		};
		void set_initial_board(const chess::Board& _board)
		{
			this->board_ = _board;
		};


		MoveTree() = default;
		MoveTree(const chess::Board& _board) :
			board_(_board)
		{};

	private:

		chess::Board board_{}; // initial board state
		std::vector<MoveTreeNode> moves_{}; // moves that can be played from the initial board state
		BoardHashSet hash_set_{};
		size_t depth_counter_ = 0;
	};




};