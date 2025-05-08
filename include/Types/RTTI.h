#pragma once

#include "Meta.h"

namespace punk
{
    // runtime type system manages all runtime information about types, components, component groups & archetypes
    class runtime_type_system
    {
    public:
        virtual ~runtime_type_system() = default;

        // factory
        static runtime_type_system* create_instance();

    public:
        // get type_info
        virtual type_info_t const* get_type_info(char const* type_name) const = 0;
        virtual type_info_t const* get_type_info(uint32_t type_name_hash) const = 0;

        // register a type info object created from meta interface
        virtual type_info_t const* register_type_info(type_info_t* type_info) = 0;

        // generic get_or_create_type_info
        template <typename T>
        type_info_t const* get_or_create_type_info()
        {
            return nullptr;
        }
    };
}