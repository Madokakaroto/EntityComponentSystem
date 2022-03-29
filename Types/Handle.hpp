#pragma once

namespace ecs
{
    template <typename Tag, std::integral T>
    struct handle
    {
    public:
        using underlying_type = T;
        using tag_type = Tag;

    public:
        underlying_type value_;

    public:
        [[nodiscard]] friend constexpr auto operator <=> (handle lhs, handle rhs) noexcept
        {
            return lhs.value_ <=> rhs.value_;
        }

        [[nodiscard]] static constexpr underlying_type invalid_handle_value() noexcept
        {
            return (std::numeric_limits<underlying_type>::max)();
        }

        [[nodiscard]] static constexpr handle invalid_handle() noexcept
        {
            return { invalid_handle_value() };
        }

        [[nodiscard]] explicit constexpr operator underlying_type() const noexcept
        {
            return value_;
        }

        [[nodiscard]] constexpr underlying_type get_value() const noexcept
        {
            return value_;
        }

        [[nodiscard]] constexpr bool is_valid() const noexcept
        {
            return value_ != invalid_handle_value();
        }
    };
}