// component.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <neogfx/neogfx.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <neolib/intrusive_sort.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_component.hpp>

namespace neogfx::game
{
	class i_ecs;

	template <typename Data>
	struct shared;

	namespace detail
	{
		template <typename Data>
		struct crack_component_data
		{
			typedef Data data_type;
			typedef Data value_type;
			typedef std::vector<value_type> container_type;
			static constexpr bool optional = false;
		};

		template <typename Data>
		struct crack_component_data<std::optional<Data>>
		{
			typedef Data data_type;
			typedef std::optional<data_type> value_type;
			typedef std::vector<value_type> container_type;
			static constexpr bool optional = true;
		};

		template <typename Data>
		struct crack_component_data<shared<Data>>
		{
			typedef Data data_type;
			typedef data_type mapped_type;
			typedef std::pair<const std::string, mapped_type> value_type;
			typedef std::unordered_map<std::string, mapped_type> container_type;
			static constexpr bool optional = false;
		};

		template <typename Data>
		struct crack_component_data<shared<std::optional<Data>>>
		{
			typedef Data data_type;
			typedef std::optional<data_type> mapped_type;
			typedef std::pair<const std::string, mapped_type> value_type;
			typedef std::unordered_map<std::string, mapped_type> container_type;
			static constexpr bool optional = true;
		};
	}

	template <typename Data, typename Base>
	class static_component_base : public Base
	{
	public:
		struct entity_record_not_found : std::logic_error { entity_record_not_found() : std::logic_error("neogfx::static_component::entity_record_not_found") {} };
		struct invalid_data : std::logic_error { invalid_data() : std::logic_error("neogfx::static_component::invalid_data") {} };
	public:
		typedef typename detail::crack_component_data<Data>::data_type data_type;
		typedef typename data_type::meta data_meta_type;
		typedef typename detail::crack_component_data<Data>::value_type value_type;
		typedef typename detail::crack_component_data<Data>::container_type component_data_t;
	private:
		typedef static_component_base<Data, Base> self_type;
	public:
		static_component_base(game::i_ecs& aEcs) : 
			iEcs{ aEcs }
		{
		}
	public:
		game::i_ecs& ecs() const override
		{
			return iEcs;
		}
		const component_id& id() const override
		{
			return data_meta_type::id();
		}
	public:
		bool is_data_optional() const override
		{
			return detail::crack_component_data<Data>::optional;
		}
		const neolib::i_string& name() const override
		{
			return data_meta_type::name();
		}
		uint32_t field_count() const override
		{
			return data_meta_type::field_count();
		}
		component_data_field_type field_type(uint32_t aFieldIndex) const override
		{
			return data_meta_type::field_type(aFieldIndex);
		}
		neolib::uuid field_type_id(uint32_t aFieldIndex) const override
		{
			return data_meta_type::field_type_id(aFieldIndex);
		}
		const neolib::i_string& field_name(uint32_t aFieldIndex) const override
		{
			return data_meta_type::field_name(aFieldIndex);
		}
	public:
		const component_data_t& component_data() const
		{
			return iComponentData;
		}
		component_data_t& component_data()
		{
			return iComponentData;
		}
		const value_type& operator[](typename component_data_t::size_type aIndex) const
		{
			return *std::next(component_data().begin(), aIndex);
		}
		value_type& operator[](typename component_data_t::size_type aIndex)
		{
			return *std::next(component_data().begin(), aIndex);
		}
	private:
		game::i_ecs& iEcs;
		component_data_t iComponentData;
	};

	template <typename Data>
	class static_component : public static_component_base<Data, i_component>
	{
	private:
		typedef static_component_base<Data, i_component> base_type;
	public:
		typedef typename base_type::data_type data_type;
		typedef typename base_type::data_meta_type data_meta_type;
		typedef typename base_type::value_type value_type;
		typedef typename base_type::component_data_t component_data_t;
		typedef std::vector<entity_id> component_data_index_t;
		typedef std::vector<component_data_index_t::size_type> reverse_index_t;
	private:
		typedef static_component<Data> self_type;
	private:
		static constexpr component_data_index_t::size_type invalid = ~component_data_index_t::size_type{};
	public:
		static_component(game::i_ecs& aEcs) : 
			base_type{ aEcs }
		{
		}
	public:
		const component_data_index_t& index() const
		{
			return iIndex;
		}
		component_data_index_t& index()
		{
			return iIndex;
		}
		const reverse_index_t& reverse_index() const
		{
			return iReverseIndex;
		}
		reverse_index_t& reverse_index()
		{
			return iReverseIndex;
		}
		bool has_entity_record(entity_id aEntity) const override
		{
			return reverse_index()[aEntity] != invalid;
		}
		const data_type& entity_record(entity_id aEntity) const
		{
			auto reverseIndex = reverse_index()[aEntity];
			if (reverseIndex == invalid)
				throw entity_record_not_found();
			return base_type::component_data()[reverseIndex];
		}
		value_type& entity_record(entity_id aEntity)
		{
			return const_cast<value_type&>(const_cast<const self_type*>(this)->entity_record(aEntity));
		}
		void destroy_entity_record(entity_id aEntity) override
		{
			auto& reverseIndex = reverse_index()[aEntity];
			if (reverseIndex == invalid)
				throw entity_record_not_found();
			if constexpr (data_meta_type::has_handles)
				data_meta_type::free_handles(base_type::component_data()[reverseIndex], ecs());
			index()[reverseIndex] = null_entity;
			reverseIndex = invalid;
		}
		value_type& populate(entity_id aEntity, const value_type& aData)
		{
			return do_populate(aEntity, aData);
		}
		value_type& populate(entity_id aEntity, value_type&& aData)
		{
			return do_populate(aEntity, aData);
		}
		void* populate(entity_id aEntity, const void* aComponentData, std::size_t aComponentDataSize) override
		{
			if ((aComponentData == nullptr && !is_data_optional()) || aComponentDataSize != sizeof(data_type))
				throw invalid_data();
			if (aComponentData != nullptr)
				return &do_populate(aEntity, *static_cast<const data_type*>(aComponentData));
			else
				return &do_populate(aEntity, value_type{}); // empty optional
		}
	public:
		template <typename Compare>
		void sort(Compare aComparator)
		{
			neolib::intrusive_sort(base_type::component_data().begin(), base_type::component_data().end(),
				[this](auto lhs, auto rhs) 
				{ 
					std::swap(*lhs, *rhs);
					auto lhsIndex = lhs - base_type::component_data().begin();
					auto rhsIndex = rhs - base_type::component_data().begin();
					std::swap(index()[lhsIndex], index()[rhsIndex]);
					std::swap(reverse_index()[index()[lhsIndex]], reverse_index()[index()[rhsIndex]]);
				}, aComparator);
		}
	private:
		template <typename T>
		value_type& do_populate(entity_id aEntity, T&& aComponentData)
		{
			if (has_entity_record(aEntity))
				return do_update(aEntity, aComponentData);
			base_type::component_data().push_back(std::forward<T>(aComponentData));
			try
			{
				index().push_back(aEntity);
			}
			catch (...)
			{
				base_type::component_data().pop_back();
				throw;
			}
			try
			{
				if (reverse_index().size() < aEntity)
					reverse_index().resize(aEntity, invalid);
				reverse_index()[aEntity] = index().size() - 1;
			}
			catch (...)
			{
				base_type::component_data().pop_back();
				index().pop_back();
			}
			return base_type::component_data().back();
		}
		template <typename T>
		value_type& do_update(entity_id aEntity, T&& aComponentData)
		{
			auto& record = entity_record(aEntity);
			record = aComponentData;
			return record;
		}
	private:
		component_data_index_t iIndex;
		reverse_index_t iReverseIndex;
	};

	template <typename Data>
	struct shared
	{
		typedef typename detail::crack_component_data<shared<Data>>::mapped_type mapped_type;
		const mapped_type* ptr;
	};

	template <typename Data>
	class static_shared_component : public static_component_base<shared<Data>, i_shared_component>
	{
	private:
		typedef static_component_base<shared<Data>, i_shared_component> base_type;
	public:
		typedef typename base_type::data_type data_type;
		typedef typename base_type::data_meta_type data_meta_type;
		typedef typename base_type::value_type value_type;
		typedef typename base_type::component_data_t component_data_t;
		typedef typename component_data_t::mapped_type mapped_type;
	private:
		typedef static_shared_component<Data> self_type;
	public:
		static_shared_component(game::i_ecs& aEcs) :
			base_type{ aEcs }
		{
		}
	public:
		const mapped_type& operator[](typename component_data_t::size_type aIndex) const
		{
			return std::next(component_data().begin(), aIndex)->second;
		}
		mapped_type& operator[](typename component_data_t::size_type aIndex)
		{
			return std::next(component_data().begin(), aIndex)->second;
		}
		const mapped_type& operator[](const std::string& aName) const
		{
			return component_data()[aName];
		}
		mapped_type& operator[](const std::string& aName)
		{
			return component_data()[aName];
		}
	public:
		mapped_type& populate(const std::string& aName, const mapped_type& aData)
		{
			base_type::component_data()[aName] = aData;
			auto& result = base_type::component_data()[aName];
			if constexpr (mapped_type::meta::has_updater)
				mapped_type::meta::update(result, ecs(), null_entity);
			return result;
		}
		mapped_type& populate(const std::string& aName, mapped_type&& aData)
		{
			base_type::component_data()[aName] = std::move(aData);
			auto& result = base_type::component_data()[aName];
			if constexpr (mapped_type::meta::has_updater)
				mapped_type::meta::update(result, ecs(), null_entity);
			return result;
		}
		void* populate(const std::string& aName, const void* aComponentData, std::size_t aComponentDataSize) override
		{
			if ((aComponentData == nullptr && !is_data_optional()) || aComponentDataSize != sizeof(mapped_type))
				throw invalid_data();
			if (aComponentData != nullptr)
				return &populate(aName, *static_cast<const mapped_type*>(aComponentData));
			else
				return &populate(aName, mapped_type{}); // empty optional
		}
	};
}