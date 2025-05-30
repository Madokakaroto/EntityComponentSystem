#pragma once

namespace punk
{
    struct chunk_t
    {
        static constexpr size_t chunke_size = 16 * 1024;

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
        type_vtable_t               vtable;
        vector<field_info_t>        fields;

        // TODO ... using C++ attributes to manager these two
        component_tag_t             component_tag;
        uint32_t                    component_group;
    };

    struct component_info_t
    {
        uint32_t                    index_in_archetype;
        uint32_t                    index_in_group;
        uint32_t                    index_of_group;
        uint32_t                    offset_in_chunk;
    };

    struct component_group_info_t
    {
        uint32_t                    hash;
        uint32_t                    capacity_in_chunk;
        uint32_t                    index_in_archetype;
        vector<uint32_t>            component_indices;      // indices of component in the owner archetype
    };

    struct archetype_t
    {
        uint32_t                        hash;
        bool                            registered;
        vector<type_info_t const*>      component_types;
        vector<component_info_t>        component_infos;
        vector<component_group_info_t>  component_groups;
    };

    using archetype_delete_delegate_t = std::function<void(archetype_t*)>;
    using component_index_t = handle<component_info_t, uint16_t>;
}