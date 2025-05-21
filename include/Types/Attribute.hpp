#pragma once
#include "Forward.hpp"
#include "Traits/TypeTraitsExt.hpp"

#define PUNK_MAKE_ATTRIBUTES(...) using attributes = std::tuple<__VA_ARGS__>
#define PUNK_ATTRIBUTE_TYPE(type) ::punk::attribute_##type##_t
#define PUNK_ATTRIBUTE_ITEM(type, value) std::pair<type, value>

#define PUNK_DEFINE_ATTRIBUTE(type) struct attribute_##type##_t {}

namespace punk
{
    template <size_t I, typename T> requires(has_attributes<T>)
    constexpr auto get_attribute() -> std::tuple_element_t<I, typename T::attributes>;

    template <size_t I, typename T> requires(!has_attributes<T>)
    constexpr void get_attribute();

    // built-in attribute
    PUNK_DEFINE_ATTRIBUTE(entity_component);
    PUNK_DEFINE_ATTRIBUTE(component_group);
}