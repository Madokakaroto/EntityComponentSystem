#pragma once

namespace ecs
{
    struct archetype_t;

    struct chunk_t
    {
        static constexpr size_t chunke_size = 16 * 1024;

        archetype_t*            archetype;
        uint32_t                archetype_hash;
        uint32_t                element_count;
        uint32_t                chunk_number;
    };

    // data index in one chunk
    using chunk_index_t = handle<chunk_t, uint32_t>;

    union type_hash_t
    {
        struct
        {
            uint32_t value1;
            uint32_t value2;
        }                       components;
        uint64_t                value;
    };

    constexpr bool operator==(type_hash_t const& lhs, type_hash_t const& rhs) noexcept
    {
        return lhs.value == rhs.value;
    }

    constexpr bool operator!=(type_hash_t const& lhs, type_hash_t const& rhs) noexcept
    {
        return lhs.value != rhs.value;
    }

    struct type_info_t;

    struct field_info_t
    {
        type_info_t*                    type;
        uint32_t                        offset;
    };

    struct archetype_vtable_t
    {
        void                          (*constructor)(void*);
        void                          (*destructor)(void*);
        void                          (*copy_func)(void*, void const*);
        void                          (*swap_func)(void*, void*);
        void                          (*move_func)(void*, void*);
    };

    struct type_info_t
    {
        uint32_t                        size;
        uint32_t                        alignment;
        std::string                     name;
        type_hash_t                     hash;
        archetype_vtable_t              vtable;
        uint32_t                        flag;
        std::vector<field_info_t>       fields;
    };

    struct component_info_t
    {
        type_hash_t                     hash;
        type_info_t*                    type;
        uint32_t                        offset;
    };

    using component_index_t = handle<component_info_t, uint16_t>;

    struct archetype_t
    {
        static constexpr size_t npos = (std::numeric_limits<size_t>::max)();

        uint32_t                        hash;
        uint32_t                        capacity_in_chunk;
        std::vector<component_info_t>   components;
    };

    using archetype_ptr = std::shared_ptr<archetype_t>;
    using archetype_weak_ptr = std::weak_ptr<archetype_t>;
}