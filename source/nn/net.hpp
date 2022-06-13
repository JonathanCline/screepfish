#pragma once

/** @file */

#include <jclib/ranges.h>
#include <jclib/algorithm.h>
#include <jclib/functional.h>

#include <iostream>

#include <span>
#include <cmath>
#include <random>
#include <ranges>
#include <vector>
#include <iomanip>
#include <numeric>
#include <numbers>
#include <algorithm>

namespace nn
{
	// Math

	float sigmoid(float x);
	double sigmoid(double x);

	inline float fit(float x)
	{
		return (sigmoid(x) - 0.5f) * 2.0f;
	};
	inline double fit(double x)
	{
		return (sigmoid(x) - 0.5) * 2.0;
	};

	inline float fit(float x, float _range)
	{
		return (sigmoid(x) - 0.5f) * (2.0f * _range);
	};
	inline double fit(double x, double _range)
	{
		return (sigmoid(x) - 0.5) * (2.0f * _range);
	};

	inline float fit(float x, float _range, float _offset)
	{
		return ((sigmoid(x) - 0.5f) * (2.0f * _range)) + _offset;
	};



	// Genetics

	using Codon = float;
	using GeneticSequence = std::vector<Codon>;

	GeneticSequence mix(const GeneticSequence& lhs, const GeneticSequence& rhs);
	GeneticSequence random(size_t _size);
	GeneticSequence mutate(const GeneticSequence& _sequence, float _mtFactor);
	GeneticSequence mutate(const GeneticSequence& _sequence);

	float sequence_difference(const GeneticSequence& lhs, const GeneticSequence& rhs);

};

namespace std
{
	inline std::ostream& operator<<(std::ostream& _ostr, const nn::GeneticSequence& _value)
	{
		for (auto& v : _value)
		{
			_ostr << std::setprecision(20) << v << '\n';
		};
		return _ostr;
	};
};




namespace nn
{
	// Neuron basic type

	/**
	 * @brief Just holds a value.
	*/
	struct NeuronNode
	{
	public:

		using value_type = float;

		constexpr value_type get() const noexcept
		{
			return this->value_;
		};
		
		constexpr void set(value_type _val) noexcept
		{
			this->value_ = _val;
		};

		constexpr NeuronNode() = default;
		constexpr explicit NeuronNode(value_type _val) noexcept :
			value_(_val)
		{};

		constexpr NeuronNode& operator=(value_type _value)
		{
			this->set(_value);
			return *this;
		};

	private:
		value_type value_ = 0;
	};



	// Net

	struct SimpleNeuronNode : public NeuronNode
	{
		using NeuronNode::NeuronNode;
		using NeuronNode::operator=;
	};

	struct SimpleNeuron : public SimpleNeuronNode
	{
		using value_type = float;
		using node = SimpleNeuronNode;

		struct Input
		{
		public:
			using node = SimpleNeuronNode;
			using value_type = float;

			auto weight() const noexcept
			{
				return this->weight_;
			};
			void set_weight(float _weight) noexcept
			{
				this->weight_ = _weight;
			};

			value_type get_unweighted() const noexcept
			{
				return this->neuron_->get();
			};
			value_type get() const noexcept
			{
				return this->get_unweighted() * this->weight();
			};

			Input() = default;
		
			node* neuron_{};
			float weight_ = 0.1f;
		};
		
		value_type calculate_value() const;

		// Returns the newly calculated value.
		value_type update();


		SimpleNeuron() = default;

		std::vector<Input> inputs_{};
		float bias_ = 0.0f;
	};


	struct SimpleNeuralNet
	{
		using node = SimpleNeuronNode;
		using neuron = SimpleNeuron;

		using value_type = neuron::value_type;

		std::vector<value_type> calculate(std::span<const value_type> _inputs) const;
		size_t parameter_count() const;
		void set_parameters(const GeneticSequence& _sequence);
		
	private:

		void connect_layers()
		{
			// Bail if no layers are defined
			if (this->layers_.empty())
			{
				return;
			};

			// Clear all existing input connections
			for (auto& _layer : this->layers_)
			{
				for (auto& _neuron : _layer)
				{
					_neuron.inputs_.clear();
				};
			};

			// Connect the input layer to the first neuron layer
			auto _layerIter = this->layers_.begin();
			for (auto& _neuron : *_layerIter)
			{
				for (auto& _inputNode : this->inputs_)
				{
					auto _input = neuron::Input();
					_input.neuron_ = &_inputNode;
					_input.weight_ = 1.0f;
					_neuron.inputs_.push_back(_input);
				};
			};

			// Connect each subsequent layer to the previous
			auto _previousLayerIter = _layerIter;
			for (auto _currentLayerIter = std::next(_layerIter);
				_currentLayerIter != this->layers_.end();
				std::advance(_currentLayerIter, 1))
			{
				for (auto& _neuron : *_currentLayerIter)
				{
					for (auto& _inputNeuron : *_previousLayerIter)
					{
						auto _input = neuron::Input();
						_input.neuron_ = &_inputNeuron;
						_input.weight_ = 1.0f;
						_neuron.inputs_.push_back(_input);
					};
				};

				_previousLayerIter = _currentLayerIter;
			};
		};

	public:


		SimpleNeuralNet() = default;
		
		explicit SimpleNeuralNet(size_t _inputCount, std::initializer_list<size_t> _layerSizes) :
			layers_(), inputs_(_inputCount, node{})
		{
			// Create the layers
			for (auto& _layerSize : _layerSizes)
			{
				this->layers_.push_back(std::vector<neuron>(_layerSize, neuron{}));
			};

			// Connect the layers
			this->connect_layers();
		};

		SimpleNeuralNet(const SimpleNeuralNet& other) :
			layers_(other.layers_),
			inputs_(other.inputs_)
		{
			// Reconnect layers
			this->connect_layers();
		};
		SimpleNeuralNet& operator=(const SimpleNeuralNet& other)
		{
			this->layers_ = other.layers_;
			this->inputs_ = other.inputs_;

			// Reconnect layers
			this->connect_layers();
			return *this;
		};

		SimpleNeuralNet(SimpleNeuralNet&& other) noexcept :
			layers_(std::move(other.layers_)),
			inputs_(std::move(other.inputs_))
		{
			// Reconnect layers
			this->connect_layers();
		};
		SimpleNeuralNet& operator=(SimpleNeuralNet&& other) noexcept
		{
			this->layers_ = std::move(other.layers_);
			this->inputs_ = std::move(other.inputs_);

			// Reconnect layers
			this->connect_layers();
			return *this;
		};

		mutable std::vector<std::vector<neuron>> layers_{};
		mutable std::vector<node> inputs_{};
	};






	/**
	 * @brief Helper for performing evolution on genetic entities.
	*/
	class Darwin
	{
	public:

		using entity_type = SimpleNeuralNet;

		struct Individual
		{
			GeneticSequence genes{};
			entity_type entity{};
			float fitness = 0.0f;

			auto operator<=>(const Individual& rhs) const
			{
				auto& lhs = *this;
				return lhs.fitness <=> rhs.fitness;
			};

			bool operator>(const Individual& rhs) const
			{
				return this->fitness > rhs.fitness;
			};

			Individual() = default;
		};

		GeneticSequence average_genetic_sequence() const
		{
			GeneticSequence _sum{};
			for (auto& _indv : this->population_)
			{
				if (_sum.size() != _indv.genes.size())
				{
					_sum.resize(_indv.genes.size());
				};
				std::ranges::transform(_sum, _indv.genes, _sum.begin(), jc::plus);
			};
			std::ranges::transform(_sum, _sum.begin(),
				jc::divide & (float)this->population_.size());
			return _sum;
		};
		float genetic_variation() const
		{
			auto _avgSequence = this->average_genetic_sequence();
			auto _differenceSum = 0.0f;
			for (auto& _indv : this->population_)
			{
				_differenceSum += sequence_difference(_indv.genes, _avgSequence);
			};
			return _differenceSum / (float)this->population_.size();
		};


		// Seeds the population with randomized individuals.
		void seed_population()
		{
			for (auto& _indv : this->population_)
			{
				auto& _entity = _indv.entity;
				_indv.genes = random(_entity.parameter_count());
				_entity.set_parameters(_indv.genes);
			};
		};

	private:

		std::vector<Individual> new_generation(float _mtFactor)
		{
			// Storage for the next generation.
			std::vector<Individual> _generation(this->generation_size_, Individual{});
			
			// Ensure we have a population to start with.
			if (this->population_.empty())
			{
				this->seed_population();
			};

			// Mutate our existing population to get the new generation.
			{
				auto it = _generation.begin();
				it = std::copy(this->population_.begin(), this->population_.begin() + 10,
					it);

				const auto _end = _generation.end();
				while (true)
				{
					if (it >= _end)
					{
						goto done_reproduction;
					};

					for (auto& _parent : this->population_)
					{
						// Reset fitness
						it->fitness = 0.0f;
						
						// Copy parent entity.
						it->entity = _parent.entity;

						// Mutate parent parameters.
						it->genes = mutate(_parent.genes, _mtFactor);
						
						// Apply mutated parameters.
						it->entity.set_parameters(it->genes);

						// Next!
						++it;
						if (it == _end)
						{
							goto done_reproduction;
						};
					};
				};
			};

		done_reproduction: // barry allen
			return _generation;
		};

	public:

		template <std::invocable<const entity_type&> RewardFn>
		requires std::same_as<std::invoke_result_t<const RewardFn&, const entity_type&>, float>
		void evolve(const RewardFn& _rewardFunction, float _mtFactor)
		{
			// Create a new generation.
			auto _generation = this->new_generation(_mtFactor);

			// Calculate the fitness for each individual.
			std::ranges::for_each(_generation, [&_rewardFunction](Individual& _indv)
				{
					// Execute the reward function.
					const auto _fitness = _rewardFunction(_indv.entity);

					// Store caculated reward value as the fitness.
					_indv.fitness = _fitness;
				});

			// Sort by fitness, higher is better and closer to the front.
			std::ranges::sort(_generation, jc::greater);

			// Cull the worst performers.
			const auto _survivorCount = static_cast<size_t>(std::lerp(0.0f, (float)_generation.size(), this->survival_factor_));
			_generation.erase(_generation.begin() + _survivorCount, _generation.end());

			// Assign the new generation to the managed population.
			this->population_ = _generation;
		};

		template <std::invocable<const entity_type&> RewardFn>
		requires std::same_as<std::invoke_result_t<const RewardFn&, const entity_type&>, float>
		void evolve(const RewardFn& _rewardFunction, float _mtFactor, size_t _generations)
		{
			for (size_t n = 0; n != _generations; ++n)
			{
				this->evolve(_rewardFunction, _mtFactor);
			};
		};


		Darwin() = default;
		explicit Darwin(size_t _generationSize, float _survivalFactor, const entity_type& _entityTemplate) :
			generation_size_(_generationSize),
			survival_factor_(_survivalFactor),
			population_(_generationSize, Individual{})
		{
			// Apply the entity template.
			for (auto& _indv : this->population_)
			{
				_indv.entity = _entityTemplate;
			};

			// Seed an initial population.
			this->seed_population();
		};


		std::vector<Individual> population_{};
		float survival_factor_ = 0.5f;
		size_t generation_size_ = 0;
	};



};