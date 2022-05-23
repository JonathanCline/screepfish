#pragma once

/** @file */

#include <nlohmann/json.hpp>

#include <tuple>
#include <string>
#include <string_view>
#include <type_traits>
#include <optional>

namespace lichess
{
	using json = nlohmann::json;

	template <typename ClassT, typename VarT>
	struct named_member
	{
	public:
		using pointer = VarT ClassT::*;
		using var_type = VarT;
		using var_ref = var_type&;
		using var_const_ref = std::remove_const_t<var_type>&;

		constexpr std::string_view name() const noexcept { return this->name_; };

		constexpr auto& get(ClassT& _ptr) const
		{
			auto& _var = this->ptr_;
			return _ptr.*_var;
		};
		constexpr auto& get(const ClassT& _ptr) const
		{
			auto& _var = this->ptr_;
			return _ptr.*_var;
		};

		constexpr auto& get(ClassT* _ptr) const
		{
			auto& _var = this->ptr_;
			return *_ptr.*_var;
		};
		constexpr auto& get(const ClassT* _ptr) const
		{
			auto& _var = this->ptr_;
			return *_ptr.*_var;
		};

		constexpr named_member(pointer _ptr, std::string_view _name) :
			name_(_name), ptr_(_ptr)
		{};

	private:
		std::string_view name_;
		pointer ptr_;
	};


	template <typename T>
	struct class_named_member_table;

	template <typename T>
	requires requires
	{
		class_named_member_table<T>::value;
	}
	constexpr inline auto class_named_member_table_v = class_named_member_table<T>::value;

	namespace impl
	{
		template <typename T>
		requires requires (const json& _json, const std::string& _name, T& _value)
		{
			_json.at(_name).get_to(_value);
		}
		inline void field_from_json_method(const json& _json, const std::string& _name, T& _value)
		{
			_json.at(_name).get_to(_value);
		};

		template <typename T>
		requires requires (const json& _json, const std::string& _name, T& _value)
		{
			field_from_json_method(_json, _name, _value);
		}
		inline void field_from_json_method(const json& _json, const std::string& _name, std::optional<T>& _value)
		{
			if (_json.contains(_name) && !_json.at(_name).is_null())
			{
				_value.emplace();
				field_from_json_method(_json, _name, *_value);
			}
			else
			{
				_value.reset();
			};
		};


		template <typename T, typename V>
		requires requires (const json& _json, T* _class, const named_member<T, V>& _info)
		{
			lichess::impl::field_from_json_method(_json, std::string(_info.name()), _info.get(_class));
		}
		inline void member_from_json(const json& _json, T* _class, const named_member<T, V>& _info)
		{
			lichess::impl::field_from_json_method(_json, std::string(_info.name()), _info.get(_class));
		};


		template <typename T, typename... Ts, size_t... Idxs>
		requires requires (const json& _json, T* _class, const std::tuple<Ts...>& _tup, std::index_sequence<Idxs...>)
		{
			(member_from_json(_json, _class, std::get<Idxs>(_tup)), ...);
		}
		inline void members_from_json_impl(const json& _json, T* _class, const std::tuple<Ts...>& _tup, std::index_sequence<Idxs...>)
		{
			(member_from_json(_json, _class, std::get<Idxs>(_tup)), ...);
		};

		template <typename T, typename... Ts> requires requires(const json& _json, T* _class, const std::tuple<Ts...>& _tup)
		{
			members_from_json_impl(_json, _class, _tup, std::make_index_sequence<sizeof...(Ts)>{});
		}
		inline void members_from_json(const json& _json, T* _class, const std::tuple<Ts...>& _tup)
		{
			return members_from_json_impl(_json, _class, _tup, std::make_index_sequence<sizeof...(Ts)>{});
		};

		template <typename T> requires requires(const json& _json, T* _class)
		{
			members_from_json(_json, _class, class_named_member_table_v<T>);
		}
		inline void members_from_json(const json& _json, T* _class)
		{
			return members_from_json(_json, _class, class_named_member_table_v<T>);
		};
	};

	namespace impl
	{
		template <typename T, typename V>
		requires requires (json& _json, const T* _class, const named_member<T, V>& _info)
		{
			_json.at(std::string(_info.name())).get_to(_info.get(_class));
		}
		inline void member_to_json(json& _json, const T* _class, const named_member<T, V>& _info)
		{
			_json[std::string(_info.name())] = _info.get(_class);
		};

		template <typename T, typename... Ts, size_t... Idxs>
		requires requires (json& _json, const T* _class, const std::tuple<Ts...>& _tup, std::index_sequence<Idxs...>)
		{
			(member_from_json(_json, _class, std::get<Idxs>(_tup)), ...);
		}
		inline void members_to_json_impl(json& _json, const T* _class, const std::tuple<Ts...>& _tup, std::index_sequence<Idxs...>)
		{
			(member_to_json(_json, _class, std::get<Idxs>(_tup)), ...);
		};

		template <typename T, typename... Ts>
		requires requires(json& _json, const T* _class, const std::tuple<Ts...>& _tup)
		{
			members_to_json_impl(_json, _class, _tup, std::make_index_sequence<sizeof...(Ts)>{});
		}
		inline void members_to_json(json& _json, const T* _class, const std::tuple<Ts...>& _tup)
		{
			return members_to_json_impl(_json, _class, _tup, std::make_index_sequence<sizeof...(Ts)>{});
		};

		template <typename T>
		requires requires(json& _json, const T* _class)
		{
			members_to_json(_json, _class, class_named_member_table_v<T>);
		}
		inline void members_to_json(json& _json, const T* _class)
		{
			return members_to_json(_json, _class, class_named_member_table_v<T>);
		};

	};

	template <typename T>
	requires requires(const json& _json, T& v)
	{
		impl::members_from_json(_json, &v);
	}
	inline void from_json(const json& _json, T& v)
	{
		return impl::members_from_json(_json, &v);
	};

	template <typename T>
	requires requires(json& _json, const T& v)
	{
		impl::members_to_json(_json, &v);
	}
	inline void to_json(json& _json, const T& v)
	{
		return impl::members_to_json(_json, &v);
	};
}