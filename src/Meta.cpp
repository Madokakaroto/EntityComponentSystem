#include "Types/RTTI.h"
#include "CoreTypes.h"
#include "Utils/Hash.hpp"

namespace punk
{
    // create type info
    type_info_t* create_type_info(type_create_info const& create_info)
    {
        auto type_info = std::make_unique<type_info_t>();
        type_info->size = create_info.size;
        type_info->alignment = create_info.alignment;
        type_info->name = create_info.type_name;
        type_info->hash.components.value1 = hash_memory(create_info.type_name, std::strlen(create_info.type_name));
        type_info->vtable = create_info.vtable;
        type_info->fields.resize(create_info.field_count);
        type_info->component_tag = create_info.component_tag;
        type_info->component_group = create_info.component_group;
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

    uint32_t get_type_name_hash(type_info_t const* type_info)
    {
        return get_type_hash(type_info).components.value1;
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

    component_tag_t get_type_component_tag(type_info_t const* type_info)
    {
        return type_info ? type_info->component_tag : component_tag_t::none;
    }

    uint32_t get_type_component_group(type_info_t const* type_info)
    {
        return type_info ? type_info->component_group : 0;
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
        return field_info ? field_info->offset : invalid_offset_value();
    }
}
