#pragma once

#include "Types/Entity.hpp"

namespace punk
{
    class entity_pool
    {
    public:
        entity_pool() = default;
        virtual ~entity_pool() = default;
        entity_pool(entity_pool const&) = delete;
        entity_pool& operator=(entity_pool const&) = delete;
        entity_pool(entity_pool&&) = delete;
        entity_pool& operator=(entity_pool&&) = delete;

        static entity_pool* create_entity_pool();

    public:
        virtual entity_t allocate_entity(uint16_t tag) = 0;
        virtual void deallocate_entity(entity_t entity) = 0;
        virtual void is_alive(entity_t entity) = 0;
        virtual entity_t restore_entity(entity_handle handle) = 0;
    };
}