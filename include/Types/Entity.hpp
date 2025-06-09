#pragma once

#include <compare>
#include "Types/Handle.hpp"

namespace punk
{
    // tagged type entity_handle: uint32_t handle value with entity_t
    using entity_handle = handle<class entity_t, uint32_t>;

    // the entity type
    class entity_t final
    {
    public:
        using value_type = uint64_t;
        using entity_tag = uint16_t;
        using entity_version = uint16_t;

        struct compose_type
        {
            entity_handle   handle;
            entity_tag      tag;
            entity_version  version;
        };

        static_assert(sizeof(compose_type) == sizeof(value_type));

    private:
        union
        {
            value_type      value_;
            compose_type    composed_value_;
        };

    public:
        entity_t() = default;
        ~entity_t() = default;
        entity_t(entity_t const&) = default;
        entity_t& operator=(entity_t const&) = default;
        entity_t(entity_t&&) = default;
        entity_t& operator=(entity_t&&) = default;

    public:
        static constexpr entity_t compose(entity_handle handle, entity_tag tag = 0, entity_version version = 0)
        {
            entity_t temp;
            temp.composed_value_ = { handle, tag, version };
            return temp;
        }

        static constexpr entity_t invalid_entity() noexcept
        {
            return compose(entity_handle::invalid_handle());
        }

        friend constexpr auto operator <=> (entity_t const& lhs, entity_t const& rhs) noexcept
        {
            return lhs.value_ <=> rhs.value_;
        }

    private:
        static constexpr compose_type invalid_entity_compose() noexcept
        {
            return { entity_handle::invalid_handle() };
        }

    public:
        constexpr bool is_valid() const noexcept
        {
            return composed_value_.handle.is_valid();
        }

        explicit constexpr operator bool() const noexcept
        {
            return is_valid();
        }

        constexpr entity_handle get_handle() const noexcept
        {
            return composed_value_.handle;
        }

        constexpr entity_tag get_tag() const noexcept
        {
            return composed_value_.tag;
        }

        constexpr entity_version get_version() const noexcept
        {
            return composed_value_.version;
        }

        constexpr value_type get_value() const noexcept
        {
            return value_;
        }

        explicit constexpr operator value_type() const noexcept
        {
            return get_value();
        }
    };
}
