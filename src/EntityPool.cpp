#include "Types/EntityPool.h"
#include "Utils/Hive.hpp"

namespace punk
{
    class entity_pool_impl : public entity_pool
    {
    public:
        virtual entity_t allocate_entity(uint16_t tag) override
        {
            return entity_t::invalid_entity();
        }

        virtual void deallocate_entity(entity_t entity) override
        {
            
        }

        virtual void is_alive(entity_t entity) override
        {
            
        }

        virtual entity_t restore_entity(entity_handle handle) override
        {
            return entity_t::invalid_entity();
        }
    };

    entity_pool* entity_pool::create_entity_pool()
    {
        return new entity_pool_impl{};
    }
}