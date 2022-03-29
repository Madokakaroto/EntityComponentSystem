#pragma once

namespace ecs
{
    struct data_component_tag {};
    struct shared_component_tag {};

    struct entity_component
    {
        using component_tag = data_component_tag;

        entity_t handle;
    };

    // check for empty base class optimization
    static_assert(sizeof(entity_component) == sizeof(entity_t));
}