#include "Types/RTTI.h"
#include "async_simple/coro/SpinLock.h"
#include "ECS.h"
#include "CoreTypes.h"
#include "Utils/Hash.hpp"

namespace punk
{
    // create type info
    type_info_t* create_type_info(char const* type_name, uint32_t size, uint32_t alignment, uint32_t type_tag, type_vtable_t const& type_vtable, uint32_t field_count)
    {
        auto type_info = std::make_unique<type_info_t>();
        type_info->size = size;
        type_info->alignment = alignment;
        type_info->name = type_name;
        type_info->hash.components.value1 = hash_memory(type_name, std::strlen(type_name));
        type_info->tag = type_tag;
        type_info->vtable = type_vtable;
        type_info->fields.resize(field_count);
        return type_info.release();
    }

    // delete type info
    void destroy_type_info(type_info_t* type_info) noexcept
    {
       if(type_info)
       {
           delete type_info;
       }
    }

    // get size
    uint32_t get_type_size(type_info_t const* type_info)
    {
       return type_info ? type_info->size : 0;
    }

    // get alignment
    uint32_t get_type_align(type_info_t const* type_info)
    {
       return type_info ? type_info->alignment : 0;
    }

    // get type name
    char const* get_type_name(type_info_t const* type_info)
    {
       return type_info ? type_info->name.c_str() : nullptr;
    }

    // get type hash
    type_hash_t get_type_hash(type_info_t const* type_info)
    {
       return type_info ? type_info->hash : type_hash_t{ uint64_t{0} };
    }

    type_tag_t get_type_tag(type_info_t const* type_info)
    {
        return type_info ? static_cast<type_tag_t>(type_info->tag) : type_tag_trivial;
    }

    // get field count
    uint32_t get_type_field_count(type_info_t const* type_info)
    {
       return type_info ? static_cast<uint32_t>(type_info->fields.size()) : 0;
    }

    field_info_t const* get_type_field_info(type_info_t const* type_info, size_t field_index)
    {
        if (!type_info)
        {
            return nullptr;
        }

        if (field_index >= type_info->fields.size())
        {
            return nullptr;
        }

        return &type_info->fields[field_index];
    }

    field_info_t* get_mutable_type_field_info(type_info_t* type_info, size_t field_index)
    {
        return const_cast<field_info_t*>(get_type_field_info(type_info, field_index));
    }

    void update_hash_for_fields(type_info_t* type_info)
    {
        std::vector<type_hash_t> all_fileds_type_hash{};
        all_fileds_type_hash.reserve(type_info->fields.size());
        std::transform(type_info->fields.begin(), type_info->fields.end(),
            std::back_inserter(all_fileds_type_hash),
            [](field_info_t const& field_info)
            {
                return field_info.type->hash;
            });
        type_info->hash.components.value2 = hash_memory(
            reinterpret_cast<char const*>(all_fileds_type_hash.data()), all_fileds_type_hash.size() * sizeof(type_hash_t));
    }
}

namespace punk
{
    void set_field_type(field_info_t* field_info, type_info_t const* field_type)
    {
        if(field_info)
        {
            field_info->type = field_type;
        }
    }

    void set_field_offset(field_info_t* field_info, uint32_t field_offset)
    {
        if(field_info)
        {
            field_info->offset = field_offset;
        }
    }

    type_info_t const* get_field_type(field_info_t* field_info)
    {
        return field_info ? field_info->type : nullptr;
    }

    uint32_t get_field_offset(field_info_t* field_info)
    {
        return field_info ? field_info->offset : invalid_offset;
    }
}

namespace punk
{
    class runtime_type_system_impl : public runtime_type_system
    {
    public:
        using spin_lock_t = async_simple::coro::SpinLock;
        using scoped_spin_lock_t = async_simple::coro::ScopedSpinLock;
        using type_info_ptr = std::unique_ptr<type_info_t>;
        using type_info_container = std::unordered_map<uint32_t, type_info_ptr>;

    private:
        mutable spin_lock_t type_locks;
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
            scoped_spin_lock_t lock{ type_locks };
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
            scoped_spin_lock_t lock{ type_locks };
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
            auto scope = type_locks.coScopedLock();
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
            auto scope = type_locks.coScopedLock();
            auto const emplace_result = runtime_type_infos.emplace(type_name_hash, type_info);
            co_return emplace_result.first->second.get();
        }
    };

    runtime_type_system* runtime_type_system::create_instance()
    {
        return new runtime_type_system_impl{};
    }
}