#include "net.hpp"

#include "utility/utility.hpp"





namespace nn
{
	template <typename T>
	inline T sigmoid_fn(T x)
	{
		constexpr auto e = std::numbers::e_v<T>;
		return	static_cast<T>(1) /
				(static_cast<T>(1) + std::pow(e, -x));
	};

	float sigmoid(float x)
	{
		return sigmoid_fn(x);
	};
	double sigmoid(double x)
	{
		return sigmoid_fn(x);
	};
};


namespace nn
{
	inline Codon random_codon()
	{
		static std::random_device rnd_{};
		static thread_local const auto sseq_ = std::seed_seq{ rnd_(), rnd_(), rnd_(), rnd_(), rnd_() };
		static thread_local std::mt19937 mt_(sseq_);
		static thread_local std::uniform_real_distribution<Codon> dist_
		{
			-1.0f, 1.0f
		};
		return dist_(mt_);
	};
	inline float random_mutator()
	{
		static std::random_device rnd_{};
		static const auto sseq_ = std::seed_seq{ rnd_(), rnd_(), rnd_(), rnd_(), rnd_() };
		static thread_local std::mt19937 mt_(sseq_);
		static thread_local std::uniform_real_distribution<Codon> dist_
		{
			-1.0f, 1.0f
		};
		return dist_(mt_);
	};

	inline auto make_codon_mutator(float _mtFactor)
	{
		return [_mtFactor](Codon _codon)
		{
			auto _mut = random_mutator();
			_mut = fit(_mut, _mtFactor);
			return _codon +=  _mut;
		};
	};
	inline Codon mutate_codon(Codon _codon)
	{
		auto _mut = random_mutator();
		_mut = fit(_mut, 0.25f);
		return _codon += _mut;
	};

	GeneticSequence mix(const GeneticSequence& lhs, const GeneticSequence& rhs)
	{
		SCREEPFISH_ASSERT(lhs.size() == rhs.size());
		auto _mixed = GeneticSequence(lhs.size());
		std::ranges::transform(lhs, rhs, _mixed.begin(), jc::times);
		return _mixed;
	};

	GeneticSequence random(size_t _size)
	{
		auto _seq = GeneticSequence(_size);
		std::ranges::generate(_seq, random_codon);
		return _seq;
	};

	GeneticSequence mutate(const GeneticSequence& _sequence, float _mtFactor)
	{
		auto _out = GeneticSequence(_sequence.size());
		std::ranges::transform(_sequence, _out.begin(), make_codon_mutator(_mtFactor));
		return _out;
	};
	GeneticSequence mutate(const GeneticSequence& _sequence)
	{
		auto _out = GeneticSequence(_sequence.size());
		std::ranges::transform(_sequence, _out.begin(), mutate_codon);
		return _out;
	};



	float sequence_difference(const GeneticSequence& lhs, const GeneticSequence& rhs)
	{
		SCREEPFISH_ASSERT(lhs.size() == rhs.size());
		auto _geneDifferences = GeneticSequence(lhs.size());
		std::ranges::transform(lhs, rhs, _geneDifferences.begin(),
			jc::minus | jc::call(&std::fabsf));
		const auto _totalDifference = jc::accumulate(_geneDifferences);
		return _totalDifference;
	};

};

namespace nn
{
	SimpleNeuron::value_type SimpleNeuron::calculate_value() const
	{
		// Function for the weighted accumulate.
		constexpr auto _accumulateFn = [](value_type s, const Input& _input) ->
			value_type
		{
			return s + _input.get();
		};

		// Aliases for future proofing.
		const auto& _inputs = this->inputs_;
		const auto& _bias = this->bias_;

		// Summate the weighted input values.
		const auto _rawInput =
			std::accumulate(_inputs.begin(), _inputs.end(), value_type{}, _accumulateFn);
		const auto _biasedInput = _rawInput + _bias;

		// Apply the sigmoid function.
		return sigmoid(_biasedInput);
	};

	SimpleNeuron::value_type SimpleNeuron::update()
	{
		const auto _newValue = this->calculate_value();
		this->set(_newValue);
		return _newValue;
	};



	std::vector<SimpleNeuralNet::value_type>
		SimpleNeuralNet::calculate(std::span<const value_type> _inputs) const
	{
		SCREEPFISH_ASSERT(_inputs.size() == this->inputs_.size());

		// Update inputs
		std::ranges::copy(_inputs, this->inputs_.begin());
		
		// Update each layer sequentially.
		for (auto& _layer : this->layers_)
		{
			// Update each neuron
			for (auto& _neuron : _layer)
			{
				_neuron.update();
			};
		};

		// Outputs will be the value held by each neuron on the last layer.
		auto _output = std::vector<value_type>(this->layers_.back().size());
		{
			auto it = _output.begin();
			for (auto& _node : this->layers_.back())
			{
				(*it++) = _node.get();
			};
		}
		return _output;
	};
	
	size_t SimpleNeuralNet::parameter_count() const
	{
		size_t n = 0;
		for (auto& _layer : this->layers_)
		{
			for (auto& _neuron : _layer)
			{
				n += static_cast<size_t>(_neuron.inputs_.size() + 1);
			};
		};
		return n;
	};
	
	void SimpleNeuralNet::set_parameters(const GeneticSequence& _sequence)
	{
		{
			const auto _parameterCount = this->parameter_count();
			SCREEPFISH_ASSERT(_sequence.size() == _parameterCount);
		};
		auto _codonIt = _sequence.begin();

		// Apply codons in the sequence to each parameter value in the net.
		for (auto& _layer : this->layers_)
		{
			for (auto& _neuron : _layer)
			{
				for (auto& _input : _neuron.inputs_)
				{
					_input.weight_ = *(_codonIt++);
				};
				_neuron.bias_ = *(_codonIt++);
			};
		};
	};


};
