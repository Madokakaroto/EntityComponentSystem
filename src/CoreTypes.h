#pragma once

namespace punk
{
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
        uint32_t                    flag;
        type_vtable_t               vtable;
        vector<field_info_t>        fields;
    };

    struct component_info_t
    {
        type_hash_t                 hash;
        type_info_t*                type;
        uint32_t                    offset;
    };

    using component_index_t = handle<component_info_t, uint16_t>;

    struct archetype_t
    {
        static constexpr size_t npos = (std::numeric_limits<size_t>::max)();

        uint32_t                    hash;
        uint32_t                    capacity_in_chunk;
        vector<component_info_t>    components;
    };

    using archetype_ptr = std::shared_ptr<archetype_t>;
    using archetype_weak_ptr = std::weak_ptr<archetype_t>;
}