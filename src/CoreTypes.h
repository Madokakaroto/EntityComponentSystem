#pragma once

namespace punk
{
    constexpr uint32_t invalid_index_value() noexcept { return (std::numeric_limits<uint32_t>::max)(); }
    constexpr uint32_t invalid_offset_value() noexcept { return invalid_index_value(); }

    struct chunk_t
    {
        static constexpr size_t chunke_size = 16 * 1024;

        archetype_t*                archetype;
        uint32_t                    archetype_hash;
        uint32_t                    element_count;
        uint32_t                    chunk_number;
    };

    // data index in one chunk
    using chunk_index_t = handle<chunk_t, uint32_t>;

    struct attribute_info_t
    {
        type_info_t const*          attribute_type;
        type_info_t const*          attribute_value;
    };

    struct field_info_t
    {
        type_info_t const*          type;
        uint32_t                    offset;
    };

    struct type_info_t
    {
        uint32_t                    size;
        uint32_t                    alignment;
        string                      name;
        type_hash_t                 hash;
        type_vtable_t               vtable;
        vector<field_info_t>        fields;
        vector<attribute_info_t>    attributes;
    };

    struct component_info_t
    {
        uint32_t                    hash;
        uint32_t                    offset_in_chunk;
    };

    struct archetype_t;
    struct component_group_info_t
    {
        archetype_t*                owner_archetype;
        uint32_t                    capacity_in_chunk;
        vector<uint32_t>            component_index;
    };

    using component_index_t = handle<component_info_t, uint16_t>;

    using archetype_delete_delegate_t = std::function<void(archetype_t*)>;
    struct archetype_t
    {
        uint32_t                        hash;
        bool                            registered;
        vector<component_info_t>        components;
        vector<component_group_info_t>  component_groups;
    };
}