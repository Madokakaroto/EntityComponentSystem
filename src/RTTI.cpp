#include "Types/RTTI.h"
#include "async_simple/coro/SpinLock.h"
#include "CoreTypes.h"
#include "Utils/Hash.hpp"

namespace punk
{
    class runtime_type_system_impl final : public runtime_type_system
    {
    public:
        using spin_lock_t = async_simple::coro::SpinLock;
        using scoped_spin_lock_t = async_simple::coro::ScopedSpinLock;
        using type_info_ptr = std::unique_ptr<type_info_t>;
        using type_info_container = std::unordered_map<uint32_t, type_info_ptr>;

    private:
        mutable spin_lock_t type_lock;
        type_info_container runtime_type_infos;

    public:
        virtual ~runtime_type_system_impl() override = default;

        virtual type_info_t* get_type_info(char const* type_name) const override
        {
            if(!type_name)
            {
                return nullptr;
            }
            auto const type_name_hash = hash_memory(type_name, std::strlen(type_name));
            return get_type_info(type_name_hash);
        }

        virtual type_info_t* get_type_info(uint32_t type_name_hash) const override
        {
            scoped_spin_lock_t lock{ type_lock };
            auto itr = runtime_type_infos.find(type_name_hash);
            if(itr != runtime_type_infos.end())
            {
                return itr->second.get();
            }
            return nullptr;
        }

        virtual type_info_t const* register_type_info(type_info_t* type_info) override
        {
            auto const type_name_hash = type_info->hash.components.value1;
            scoped_spin_lock_t lock{ type_lock };
            auto const emplace_result = runtime_type_infos.emplace(type_name_hash, type_info);
            return emplace_result.first->second.get();
            // TODO ... conflict when hash.component.value2 is not the same
        }

        virtual Lazy<type_info_t const*> async_get_type_info(char const* type_name) const override
        {
            if(!type_name)
            {
                co_return nullptr;
            }
            auto const type_name_hash = hash_memory(type_name, std::strlen(type_name));
            co_return co_await async_get_type_info(type_name_hash);
        }

        virtual Lazy<type_info_t const*> async_get_type_info(uint32_t type_name_hash) const override
        {
            auto scope = type_lock.coScopedLock();
            auto itr = runtime_type_infos.find(type_name_hash);
            if (itr != runtime_type_infos.end())
            {
                co_return itr->second.get();
            }
            co_return nullptr;
        }

        virtual Lazy<type_info_t const*> async_register_type_info(type_info_t* type_info) override
        {
            auto const type_name_hash = type_info->hash.components.value1;
            auto scope = type_lock.coScopedLock();
            auto const emplace_result = runtime_type_infos.emplace(type_name_hash, type_info);
            co_return emplace_result.first->second.get();
        }
    };

    runtime_type_system* runtime_type_system::create_instance()
    {
        return new runtime_type_system_impl{};
    }
}

namespace punk
{
    archetype_ptr runtime_archetype_system::get_or_create_archetype(type_info_t const** component_type_infos, size_t count)
    {
        if (count == 0)
        {
            return nullptr;
        }

        if (std::any_of(component_type_infos, component_type_infos + count,
            [=](auto const* type_info)
            {
                return get_type_component_tag(type_info) == component_tag_t::none;
            }))
        {
            return nullptr;
        }

        std::stable_sort(component_type_infos, component_type_infos + count,
            [](auto const* lhs, auto const* rhs)
            {
                return get_type_hash(lhs) < get_type_hash(rhs);
            });
        return get_or_create_archetype_impl(component_type_infos, count);
    }

    class runtime_archetype_system_impl final : public runtime_archetype_system
    {
    public:
        using spin_lock_t = async_simple::coro::SpinLock;
        using scoped_spin_lock_t = async_simple::coro::ScopedSpinLock;
        using archetype_container = std::unordered_map<uint32_t, archetype_weak>;

    private:
        archetype_container all_archetypes;
        spin_lock_t         archetype_lock;

    public:
        explicit runtime_archetype_system_impl(runtime_type_system* runtime_type_system)
            : runtime_archetype_system(runtime_type_system) {}

    public:
        virtual archetype_ptr get_archetype(uint32_t hash) override
        {
            scoped_spin_lock_t lock{ archetype_lock };
            auto itr = all_archetypes.find(hash);
            if (itr != all_archetypes.end())
            {
                auto result = itr->second.lock();
                return result;
            }
            return nullptr;
        }

        virtual archetype_ptr archetype_include_components(archetype_ptr const& archetype, type_info_t const** component_type_infos, size_t count) override
        {
            return nullptr;
        }

        virtual archetype_ptr archetype_exclude_components(archetype_ptr const& archetype, type_info_t const** component_type_infos, size_t count) override
        {
            return nullptr;
        }

    protected:
        virtual archetype_ptr get_or_create_archetype_impl(type_info_t const** component_type_infos, size_t count) override
        {
            // calculate the sorted components hash, as the archetype hash value
            std::vector<type_hash_t> hash{};
            hash.reserve(count);
            std::transform(component_type_infos, component_type_infos + count, std::back_inserter(hash),
                [](auto const* type_info)
                {
                    return get_type_hash(type_info);
                });
            auto const archetype_hash = hash_memory(reinterpret_cast<char const*>(hash.data()), sizeof(type_hash_t) * hash.size());

            // if found one, return
            auto archetype = get_archetype(archetype_hash);
            if(archetype)
            {
                return archetype;
            }

            // allocate a new 
            archetype = allocate_archetype(archetype_hash, count);

            // initialize archetype info
            initialize_archetype(archetype.get(), component_type_infos, count);

            // register archetype, two phrase commit
            archetype = register_archetype(archetype);
            return archetype;
        }

    private:
        archetype_ptr allocate_archetype(uint32_t hash, size_t component_count)
        {
            archetype_ptr archetype = archetype_ptr
            {
                new archetype_t{}, [this](archetype_t* archetype) { destroy_archetype(archetype); }
            };
            archetype->hash = 0;
            archetype->registered = false;
            archetype->components.reserve(component_count);
            archetype->component_groups.reserve(component_count);
            return archetype;
        }

        void destroy_archetype(archetype_t* archetype)
        {
            if(archetype)
            {
                if(archetype->registered)
                {
                    unregister_archetype(archetype);
                    archetype->registered = false;
                }

                delete archetype;
            }
        }

        archetype_ptr register_archetype(archetype_ptr& archetype)
        {
            assert(archetype);
            scoped_spin_lock_t lock{ archetype_lock };
            auto [weak_archetype, result] = all_archetypes.emplace(archetype->hash, archetype);
            if(result)
            {
                return archetype;
            }
            auto result_archetype = weak_archetype->second.lock();
            assert(result_archetype);
            return result_archetype;
        }

        void unregister_archetype(archetype_t* archetype)
        {
            scoped_spin_lock_t lock{ archetype_lock };
            auto itr = all_archetypes.find(archetype->hash);
            if(itr != all_archetypes.end())
            {
                all_archetypes.erase(itr);
            }
        }

        void initialize_archetype(archetype_t* archetype, type_info_t const** component_type_infos, size_t count)
        {
            /// initialize components info
            auto all_components_info = std::ranges::subrange
            {
                component_type_infos,
                component_type_infos + count
            };
            std::ranges::copy(all_components_info | std::views::transform(
                [index{ 0u }](auto const* comp_type_info) mutable
                {
                    return component_info_t
                    {
                        .type_info = comp_type_info,
                        .index_in_archetype = index++,
                        .index_in_group = invalid_index_value(),
                        .group_index = invalid_index_value(),
                        .offset_in_chunk = 0,
                    };
                }), std::back_inserter(archetype->components));

            /// initialize component groups info
            // map to array of component_group_info_t
            std::ranges::transform(archetype->components, std::back_inserter(archetype->component_groups),
                [archetype](auto const& component_info)
                {
                    component_group_info_t component_group
                    {
                        .owner_archetype = archetype,
                        .hash = component_info.type_info->component_group,
                        .capacity_in_chunk = 0,
                        .component_indices = { component_info.index_in_archetype },
                    };
                    return component_group;
                });
            // sort component group by has value 
            std::ranges::stable_sort(archetype->component_groups,
                [](auto const& lhs, auto const& rhs)
                {
                    return lhs.hash < rhs.hash;
                });
            // unique all the duplicated components, and merge the component indices
            auto [erase_begin, erase_last] = std::ranges::unique(archetype->component_groups,
                [](component_group_info_t& lhs, component_group_info_t& rhs)
                {
                    if(lhs.hash == rhs.hash)
                    {
                        lhs.component_indices.insert_range(lhs.component_indices.end(), rhs.component_indices);
                        return true;
                    }
                    return false;
                });
            archetype->component_groups.erase(erase_begin, erase_last);
            // update the component group index for all component
            std::ranges::for_each(archetype->component_groups,
                [index{ 0u }, archetype](auto& component_group) mutable
                {
                    component_group.index = index++;
                    std::ranges::for_each(component_group.component_indices,
                        [index{ 0u }, group_index = component_group.index, archetype](uint32_t component_index) mutable
                        {
                            auto& component_info = archetype->components[component_index];
                            component_info.group_index = group_index;
                            component_info.group_index = index++;
                        });
                });

            // initialize memory capacity_in_chunk for component_group & offset_in_chunk for component
            search_chunck_offset_and_capacity(archetype);
        }

        void search_chunck_offset_and_capacity(archetype_t* archetype)
        {
            assert(archetype);

            for(auto& component_group : archetype->component_groups)
            {
                /// Search the capcity in chunk for each component group
                // 1. accumulate all the component size
                auto const all_comp_size = std::reduce(
                    component_group.component_indices.begin(),
                    component_group.component_indices.end(),
                    0u, [archetype](uint32_t acc, uint32_t compoent_index)
                    {
                        assert(compoent_index < archetype->components.size());
                        auto const& component_info = archetype->components[compoent_index];
                        return acc + component_info.type_info->size;
                    });

                // 2. We search the capacity with a initialize value of (data_block_size / all_comp_size + 1)
                constexpr uint32_t data_block_size = chunk_t::chunke_size - sizeof(chunk_t);
                uint32_t capacity = data_block_size / all_comp_size + 1;

                // 3. search the best capacity, by considering the alignments
                uint32_t chunk_size;
                std::vector<uint32_t> offsets(component_group.component_indices.size(), 0u);
                do
                {
                    capacity--;
                    chunk_size = calculate_chunk_size_and_offsets(archetype, component_group, capacity, offsets);
                } while (data_block_size <= chunk_size);

                // 4. now we have got the capacity result and offsets of all components, update into the archetype/component info
                component_group.capacity_in_chunk = capacity;
                for(uint32_t loop = 0; loop < offsets.size(); ++loop)
                {
                    archetype->components[loop].offset_in_chunk = offsets[loop];
                }
            }
        }

        uint32_t calculate_chunk_size_and_offsets(archetype_t* archetype, component_group_info_t const& component_group, uint32_t capacity, std::vector<uint32_t>& offsets)
        {
            assert(archetype);
            uint32_t size = sizeof(chunk_t);

            offsets.clear();
            std::ranges::transform(component_group.component_indices, std::back_inserter(offsets),
                [archetype, &size, capacity](auto const& component_index)
                {
                    auto const& component_info = archetype->components[component_index];
                    auto const* component_type_info = component_info.type_info;

                    // adjust alignment for each component
                    auto const offset = align_up(size, component_type_info->alignment);

                    // accumulate chunk memory size
                    size += component_type_info->size * capacity;

                    // return the offset
                    return offset;
                });

            return size;
        }
    };

    runtime_archetype_system* runtime_archetype_system::create_instance(runtime_type_system* rtt_system)
    {
        assert(rtt_system);
        if(!rtt_system)
        {
            return nullptr;
        }
        return new runtime_archetype_system_impl{ rtt_system };
    }
}