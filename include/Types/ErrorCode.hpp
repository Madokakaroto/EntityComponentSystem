#pragma once

namespace punk
{
    enum class error_code
    {
        succeed                     = 0,
        entity_expired              = -1,
        component_already_exists    = -2,
        component_not_exists        = -3,
        invalid_archetype           = -4,
        archetype_count_overflow    = -5,
        index_overflow              = -6,
    };
}