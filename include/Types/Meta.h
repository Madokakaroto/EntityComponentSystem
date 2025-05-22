#pragma once

#include "Types/Forward.hpp"
#include "Types/Handle.hpp"

namespace punk
{
    constexpr uint32_t invalid_offset = (std::numeric_limits<uint32_t>::max)();
    constexpr uint32_t invalid_size = (std::numeric_limits<uint32_t>::max)();;

    union type_hash_t
    {
        struct
        {
            // value1 is the hash value of type name
            uint32_t value1;
            // value2 is the hash value of all field member`s hash value
            uint32_t value2;
        } components;
        uint64_t value;
    };

    constexpr auto operator<=>(type_hash_t const& lhs, type_hash_t const& rhs) noexcept
    {
        return lhs.value <=> rhs.value;
    }

    struct type_vtable_t
    {
        void(*constructor)(void*);
        void(*destructor)(void*);
        void(*copy_func)(void*, void const*);
        void(*swap_func)(void*, void*);
        void(*move_func)(void*, void*);
    };

    // chunk is a list of chained memroy block, where the data is actually placed
    struct chunk_t;

    // archetype is a combination of serveral unique component
    struct archetype_t;

    // runtime information about a cpp type
    struct type_info_t;

    // runtime information about a member in a specific cpp type
    struct field_info_t;

    // runtime information about an attribute in a specific cpp type
    struct attribute_info_t;

    // runtime information about an entity component
    struct component_info_t;

    // a group of entity components
    struct component_group_t;

    // TODO ... abi
    using archetype_ptr = std::shared_ptr<archetype_t>;
    using archetype_weak = std::weak_ptr<archetype_t>;
}

/// TODO ... not all the interfaces below are public, hide the implementation specific ones
// interfaces for type_info_t
namespace punk
{
    // create type info
    type_info_t* create_type_info(
        char const* type_name,
        uint32_t size,
        uint32_t alignment,
        type_vtable_t const& type_vtable,
        uint32_t field_count,
        uint32_t attribute_count);

    // delete type info
    void destroy_type_info(type_info_t* type_info) noexcept;

    // get size
    uint32_t get_type_size(type_info_t const* type_info);

    // get alignment
    uint32_t get_type_align(type_info_t const* type_info);

    // get type name
    char const* get_type_name(type_info_t const* type_info);

    // get type hash
    type_hash_t get_type_hash(type_info_t const* type_info);

    // get field count
    uint32_t get_type_field_count(type_info_t const* type_info);

    // get field info
    field_info_t const* get_type_field_info(type_info_t const* type_info, size_t field_index);
    field_info_t* get_mutable_type_field_info(type_info_t* type_info, size_t field_index);

    // get attribute count
    uint32_t get_type_attribute_count(type_info_t const* type_info);

    // get attribute info
    attribute_info_t const* query_type_attribute_info(type_info_t const* type_info, type_info_t const* attribute_type_info);
    attribute_info_t const* get_type_attribute_info(type_info_t* type_info, size_t attribute_index);
    attribute_info_t* get_mutable_type_attribute_info(type_info_t* type_info, size_t attribute_index);

    // set hash for fields
    void update_hash_for_fields(type_info_t* type_info);
}

// interfaces for field_info_t
namespace punk
{
    // set the type of the field
    void set_field_type(field_info_t* field_info, type_info_t const* field_type);

    // set the offset of the field
    void set_field_offset(field_info_t* field_info, uint32_t field_offset);

    // get the type of the field
    type_info_t const* get_field_type(field_info_t* field_info);

    // get the offset of the field
    uint32_t get_field_offset(field_info_t* field_info);
}

// interface for attribute
namespace punk
{
    type_info_t const* get_attribute_type(attribute_info_t const* attribute_info);

    void set_attribute_type(attribute_info_t* attribute_info, type_info_t const* attribute_type);

    type_info_t const* get_attribute_value(attribute_info_t const* attribute_info);

    void set_attribute_value(attribute_info_t* attribute_info, type_info_t const* attribute_value);
}