#pragma once

#include "Meta.h"
#include "Utils/StaticFor.hpp"
#include "Traits/TypeInfoTraits.hpp"
#include "async_simple/coro/Lazy.h"

namespace punk
{
    template <typename T>
    using Lazy = async_simple::coro::Lazy<T>;

    // runtime type system manages all runtime information about types, components, component groups & archetypes
    class runtime_type_system
    {
    public:
        struct type_info_deleter
        {
            void operator()(type_info_t* type_info) const noexcept
            {
                destroy_type_info(type_info);
            }
        };
        using type_info_ptr = std::unique_ptr<type_info_t, type_info_deleter>;

    public:
        runtime_type_system() = default;
        runtime_type_system(runtime_type_system const&) = delete;
        runtime_type_system& operator=(runtime_type_system const&) = delete;
        runtime_type_system(runtime_type_system&&) = delete;
        runtime_type_system& operator=(runtime_type_system&&) = delete;
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
            using type_info_traits_t = type_info_traits<T>;
            auto const type_name = type_info_traits_t::get_type_name();
            auto const type_name_hash = hash_memory(type_name.c_str(), type_name.length());

            // query exist type info
            auto* type_info = get_type_info(type_name_hash);
            if (type_info)
            {
                return type_info;
            }

            // create a new type info
            type_info_ptr new_type_info
            {
                create_type_info(
                    type_name.c_str(),
                    type_info_traits_t::get_size(),
                    type_info_traits_t::get_alignment(),
                    type_info_traits_t::get_tag(),
                    type_info_traits_t::get_vtable(),
                    type_info_traits_t::get_field_count())
            };

            // initialize field
            if constexpr (type_info_traits_t::get_field_count() > 0)
            {
                //initialize_field_info<T>(new_type_info.get(), std::make_index_sequence<type_info_traits_t::get_field_count()>{});
                static_for<0, type_info_traits_t::get_field_count()>(
                    [&]<size_t Index>()
                {
                    // query field info
                    auto* field_info = get_mutable_type_field_info(new_type_info.get(), Index);
                    assert(field_info);

                    // set type_info
                    using field_type = decltype(type_info_traits_t::template get_field_type<Index>());
                    auto* field_type_info = get_or_create_type_info<field_type>();
                    assert(field_type_info);
                    set_field_type(field_info, field_type_info);

                    // set type offset
                    auto const offset = type_info_traits_t::template get_field_offset<Index>();
                    set_field_offset(field_info, offset);
                });

                // initialize type hash component2
                update_hash_for_fields(new_type_info.get());
            }

            // 2-phrase commit
            type_info = register_type_info(new_type_info.get());
            if (type_info == new_type_info.get())
            {
                new_type_info.release();
            }
            return type_info;
        }

    public: // co-routine interface
        virtual Lazy<type_info_t const*> async_get_type_info(char const* type_name) const = 0;
        virtual Lazy<type_info_t const*> async_get_type_info(uint32_t type_name_hash) const = 0;
        virtual Lazy<type_info_t const*> async_register_type_info(type_info_t* type_info) = 0;
        
        template <typename T>
        Lazy<type_info_t const*> async_get_or_create_type_info()
        {
            using type_info_traits_t = type_info_traits<T>;
            auto const type_name = type_info_traits_t::get_type_name();
            auto const type_name_hash = hash_memory(type_name.c_str(), type_name.length());

            // query exist type info
            auto* type_info = co_await async_get_type_info(type_name_hash);
            if (type_info)
            {
                co_return type_info;
            }

            // create a new type info
            type_info_ptr new_type_info
            {
                create_type_info(
                    type_name.c_str(),
                    type_info_traits_t::get_size(),
                    type_info_traits_t::get_alignment(),
                    type_info_traits_t::get_tag(),
                    type_info_traits_t::get_vtable(),
                    type_info_traits_t::get_field_count())
            };

            // initialize field
            if constexpr (type_info_traits_t::get_field_count() > 0)
            {
                co_await async_static_for<0, type_info_traits_t::get_field_count()>(
                    [&]<size_t Index>() -> Lazy<void>
                {
                    // query field info
                    auto* field_info = get_mutable_type_field_info(new_type_info.get(), Index);
                    assert(field_info);

                    // set type_info
                    using field_type = decltype(type_info_traits_t::template get_field_type<Index>());
                    auto* field_type_info = co_await async_get_or_create_type_info<field_type>();
                    assert(field_type_info);
                    set_field_type(field_info, field_type_info);

                    // set type offset
                    auto const offset = type_info_traits_t::template get_field_offset<Index>();
                    set_field_offset(field_info, offset);
                });

                // initialize type hash component2
                update_hash_for_fields(new_type_info.get());
            }

            // 2-phrase commit
            type_info = co_await async_register_type_info(new_type_info.get());
            if (type_info == new_type_info.get())
            {
                new_type_info.release();
            }
            co_return type_info;
        }
    };
}

namespace punk
{
    class runtime_archetype_system
    {
    public:
        runtime_archetype_system(runtime_archetype_system const&) = delete;
        runtime_archetype_system& operator=(runtime_archetype_system const&) = delete;
        runtime_archetype_system(runtime_archetype_system&&) = delete;
        runtime_archetype_system& operator=(runtime_archetype_system&&) = delete;
        virtual ~runtime_archetype_system() = default;

        // factory
        static runtime_archetype_system* create_instance(runtime_type_system* rtt_system);

    public:
        virtual archetype_t const* get_archetype(uint32_t hash) = 0;
        virtual archetype_t const* get_or_create_archetype(type_info_t const** component_type_infos, size_t count) = 0;
        virtual archetype_t const* archetype_include_components(archetype_t const* archetype, type_info_t const** component_type_infos, size_t count) = 0;
        virtual archetype_t const* archetype_exclude_components(archetype_t const* archetype, type_info_t const** component_type_infos, size_t count) = 0;

    protected:
        runtime_archetype_system* runtime_type_system_;
    };
}