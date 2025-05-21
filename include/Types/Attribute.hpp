#pragma once
#include "Forward.hpp"
#include "Traits/TypeTraitsExt.hpp"

#define PUNK_DEFINE_ATTRIBUTES(...) using attributes = std::tuple<__VA_ARGS__>
#define PUNK_ATTRIBUTE_ITEM(type, value) std::pair<struct attribute_##type, struct value>
#define PUNK_ATTRIBUTE(type) attribute_##type

namespace punk
{
    template <size_t I, typename T> requires(has_attributes<T>)
    constexpr auto get_attribute() -> std::tuple_element_t<I, typename T::attributes>;

    template <size_t I, typename T> requires(!has_attributes<T>)
    constexpr void get_attribute();
}