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

            // TODO ... initialize archetype info

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
            archetype->components.resize(component_count);
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