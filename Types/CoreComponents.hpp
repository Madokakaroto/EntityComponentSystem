#pragma once

namespace punk
{
    struct data_comp_tag {};        // tag type for data component
    struct cow_comp_tag {};         // tag type for copy-on-write component

    struct entity_component
    {
        using component_tag = data_comp_tag;

        entity_t handle;
    };

    // check for empty base class optimization
    static_assert(sizeof(entity_component) == sizeof(entity_t));
}