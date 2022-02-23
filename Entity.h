#pragma once

namespace ecs
{
    class entity_t
    {
    public:
        using value_type = std::uint64_t;
        using entity_tag = std::uint16_t;
        using entity_version = std::uint16_t;
        using handle_type = handle<entity_t, std::uint32_t>;

        struct compose_type
        {
            handle_type     handle;
            entity_tag      tag;
            entity_version  version;
        };

        static_assert(sizeof(compose_type) == sizeof(value_type));

    private:
        union
        {
            value_type      value_;
            compose_type    composed_value_ = invalid_entity_compose();
        };

    public:
        [[nodiscard]] static constexpr entity_t compose(handle_type handle, entity_tag tag = 0, entity_version version = 0)
        {
            entity_t temp;
            temp.composed_value_ = { handle, tag, version };
            return temp;
        }

        [[nodiscard]] static constexpr entity_t invalid_entity() noexcept
        {
            return compose(handle_type::invalid_handle());
        }

        [[nodiscard]] friend constexpr auto operator <=> (entity_t const& lhs, entity_t const& rhs) noexcept
        {
            return lhs.value_ <=> rhs.value_;
        }

    private:
        [[nodiscard]] static constexpr compose_type invalid_entity_compose() noexcept
        {
            return { handle_type::invalid_handle() };
        }

    public:
        [[nodiscard]] constexpr bool is_valid() const noexcept
        {
            return composed_value_.handle.is_valid();
        }

        [[nodiscard]] explicit constexpr operator bool() const noexcept
        {
            return is_valid();
        }

        [[nodiscard]] constexpr handle_type handle() const noexcept
        {
            return composed_value_.handle;
        }

        [[nodiscard]] constexpr entity_tag tag() const noexcept
        {
            return composed_value_.tag;
        }

        [[nodiscard]] constexpr entity_version version() const noexcept
        {
            return composed_value_.version;
        }

        [[nodiscard]] constexpr value_type value() const noexcept
        {
            return value_;
        }

        [[nodiscard]] explicit constexpr operator value_type() const noexcept
        {
            return value();
        }
    };

    struct entity_hash_t
    {
        [[nodiscard]] size_t operator() (entity_t const& entity) const noexcept
        {
            return std::hash<entity_t::value_type>{}(entity.value());
        }
    };
}
