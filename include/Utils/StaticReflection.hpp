#pragma once
#include "boost/preprocessor.hpp"
#include "boost/pfr.hpp"
#include "Traits/TypeTraitsExt.hpp"

// type reflect info
namespace punk
{
    template <typename T>
    struct reflect_info;

    template <typename T>
    concept reflected = requires
    {
        // type
        std::same_as<T, typename reflect_info<T>::type>;

        // name
        { reflect_info<T>::name() } -> std::same_as<std::string_view>;

        // field count
        { reflect_info<T>::field_count() } -> std::convertible_to<size_t>;
        
        // members
        { reflect_info<T>::members() };
        
        // member names
        { reflect_info<T>::member_names() };
    };

    template <typename T>
    concept auto_reflectable = std::is_aggregate_v<T>;

    template <typename T>
    concept reflectable = reflected<T> || auto_reflectable<T>;

    template <size_t I, typename T> requires reflected<std::remove_cvref_t<T>>
    constexpr decltype(auto) get(T&& t) noexcept
    {
        using reflect_info_t = reflect_info<std::remove_cvref_t<T>>;
        return (std::forward<T>(t).*std::get<I>(reflect_info_t::members()));
    }

    template <size_t I, typename T> requires reflected<T>
    constexpr auto get_name() noexcept
    {
        using reflect_info_t = reflect_info<T>;
        return std::get<I>(reflect_info_t::member_names());
    }

    template <size_t I, typename T> requires reflected<T>
    struct tuple_element
    {
        using reflect_info_t = reflect_info<T>;
        using type = decltype(owner_type_of_pmd(std::get<I>(reflect_info_t::members())));
    };
    template <size_t I, typename T>
    using tuple_element_t = typename tuple_element<I, T>::type;

    struct reflected_policy
    {
        template <size_t I, typename T>
        static decltype(auto) get(T&& t) noexcept
        {
            return punk::get<I>(std::forward<T>(t));
        }

        template <size_t I, typename T>
        static auto get_name() noexcept
        {
            return punk::get_name<I, T>();
        }

        template <typename T>
        static constexpr auto field_count() noexcept
        {
            using reflect_info_t = reflect_info<T>;
            return reflect_info_t::field_count();
        }
    };

    struct auto_reflectable_policy
    {
        template <size_t I, typename T>
        static decltype(auto) get(T&& t) noexcept
        {
            return boost::pfr::get<I>(std::forward<T>(t));
        }

        template <size_t I, typename T>
        static auto get_name() noexcept
        {
            return boost::pfr::get_name<I, T>();
        }

        template <typename T>
        static constexpr auto field_count() noexcept
        {
            return boost::pfr::tuple_size_v<T>;
        }
    };
}

// attribute type
#define PUNK_ATTRIBUTE_TYPE_2(ns, type) ns::attribute_##type##_t
#define PUNK_ATTRIBUTE_TYPE_1(type) PUNK_ATTRIBUTE_TYPE_2(punk, type)
#define PUNK_ATTRIBUTE_TYPE(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(PUNK_ATTRIBUTE_TYPE_, __VA_ARGS__)(__VA_ARGS__), BOOST_PP_EMPTY())
#define PUNK_DEFINE_ATTRIBUTE_TYPE(type) struct attribute_##type##_t{};
// attribute value
#define PUNK_ATTRIBUTE_VALUE_2(ns, value) ns::value
#define PUNK_ATTRIBUTE_VALUE_1(value) PUNK_ATTRIBUTE_VALUE_2(punk, value)
#define PUNK_ATTRIBUTE_VALUE(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(PUNK_ATTRIBUTE_VALUE_, __VA_ARGS__)(__VA_ARGS__), BOOST_PP_EMPTY())
// attribute item type_attribute<attribute_type, attribute_value>
#define PUNK_ATTRIBUTE_ITEM_4(nst, type, nsv, value) std::pair<PUNK_ATTRIBUTE_TYPE(nst, type) BOOST_PP_COMMA() PUNK_ATTRIBUTE_VALUE(nsv, value)>
#define PUNK_ATTRIBUTE_ITEM_3(ns, type, value) PUNK_ATTRIBUTE_ITEM_4(ns, type, ns, value)
#define PUNK_ATTRIBUTE_ITEM_2(type, value) PUNK_ATTRIBUTE_ITEM_3(punk, type, value)
#define PUNK_ATTRIBUTE_ITEM(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(PUNK_ATTRIBUTE_ITEM_, __VA_ARGS__)(__VA_ARGS__), BOOST_PP_EMPTY())
// attributes
#define PUNK_MAKE_ATTRIBUTE(r, data, i, t) BOOST_PP_COMMA_IF(i) PUNK_ATTRIBUTE_ITEM(BOOST_PP_TUPLE_REM_CTOR(t))
#define PUNK_MAKE_ATTRIBUTES(...) using attributes = std::tuple<BOOST_PP_SEQ_FOR_EACH_I(PUNK_MAKE_ATTRIBUTE, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))>

// attribute info
namespace punk
{
    template <typename T>
    struct attribute_info;

    template <typename T> requires(has_attributes<attribute_info<T>>)
    constexpr uint32_t attribute_count() noexcept
    {
        return std::tuple_size_v<typename attribute_info<T>::attributes>;
    }

    template <typename T> requires(!has_attributes<attribute_info<T>>)
    constexpr uint32_t attribute_count() noexcept
    {
        return 0;
    }

    template <size_t I, typename T> requires(has_attributes<attribute_info<T>>)
    constexpr auto attribute_type() noexcept -> std::tuple_element_t<I, typename attribute_info<T>::attributes>;

    template <size_t I, typename T> requires(!has_attributes<attribute_info<T>>)
    constexpr void attribute_type() noexcept;

    PUNK_DEFINE_ATTRIBUTE_TYPE(component_tag);
    struct data_component_tag_t{};
    struct cow_component_tag_t{};
    PUNK_DEFINE_ATTRIBUTE_TYPE(component_group);
    struct default_component_group_t{};
}

#define PUNK_ATTRIBUTE(CLASS, ...) \
template <> struct ::punk::attribute_info<CLASS> \
{\
    PUNK_MAKE_ATTRIBUTES(__VA_ARGS__); \
};

// implementation detail for PUNK_REFLECT_MEMBERS
#define PUNK_REFLECT_APPLY_MACRO(macro) macro
#define PUNK_REFLECT_ACCESS_MEMBER(c, m) &c::m
#define PUNK_REFLECT_MEMBER_NAME_II(member) std::string_view{ #member }
#define PUNK_REFLECT_MEMBER_NAME_I(member) PUNK_REFLECT_MEMBER_NAME_II(member)
#define PUNK_REFLECT_MEMBER_OFFSET_I(c, m) offsetof(c, m)
#define PUNK_REFLECT_MEMBER_NAME(r, data, i, t) BOOST_PP_COMMA_IF(i) PUNK_REFLECT_MEMBER_NAME_I(t)
#define PUNK_REFLECT_MEMBER_DATA(r, data, i, t) BOOST_PP_COMMA_IF(i) PUNK_REFLECT_ACCESS_MEMBER(data, t)
#define PUNK_REFLECT_MEMBER_OFFSET(r, data, i, t) BOOST_PP_COMMA_IF(i) PUNK_REFLECT_MEMBER_OFFSET_I(data, t)

// members generation for reflect_info
#define PUNK_REFLECT_MEMBERS(CLASS, N, ...)                                                                                  \
using type = CLASS;                                                                                                          \
static constexpr std::string_view name() noexcept { return #CLASS; }                                                         \
static constexpr std::size_t field_count() noexcept { return N; }                                                            \
static constexpr auto member_names() noexcept -> std::array<std::string_view, N>                                             \
{                                                                                                                            \
    return { BOOST_PP_SEQ_FOR_EACH_I(PUNK_REFLECT_MEMBER_NAME, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) };                  \
}                                                                                                                            \
static constexpr auto members() noexcept                                                                                     \
{                                                                                                                            \
    return std::make_tuple(BOOST_PP_SEQ_FOR_EACH_I(PUNK_REFLECT_MEMBER_DATA, CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))); \
}                                                                                                                            \
static constexpr auto member_offsets() noexcept -> std::array<size_t, N>                                                     \
{                                                                                                                            \
    return { BOOST_PP_SEQ_FOR_EACH_I(PUNK_REFLECT_MEMBER_OFFSET, CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) };            \
}

// the reflection interface
#define PUNK_REFLECT(CLASS, ...)                                                  \
template <> struct ::punk::reflect_info<CLASS>                                    \
{                                                                                 \
    static_assert(std::negation_v<std::conjunction<                               \
        std::is_pointer<CLASS>,                                                   \
        std::is_reference<CLASS>,                                                 \
        std::is_array<CLASS>,                                                     \
        std::is_const<CLASS>,                                                     \
        std::is_const<CLASS>,                                                     \
        std::is_volatile<CLASS>>>);                                               \
    PUNK_REFLECT_MEMBERS(CLASS, BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), __VA_ARGS__) \
};

// implementations & interfaces to access member
namespace punk
{
    namespace detail
    {
        template <size_t I, typename Policy, typename T, typename F>
        inline void for_each_field(Policy, T&& t, F&& f)
        {
            using func_type = std::decay_t<std::remove_cvref_t<F>>;
            using element_type = decltype(Policy::template get<I>(std::forward<T>(t)));
            if constexpr(std::is_invocable_v<func_type, element_type>)
            {
                std::invoke(std::forward<F>(f), Policy::template get<I>(std::forward<T>(t)));
            }
            else if constexpr(std::is_invocable_v<func_type, element_type, size_t>)
            {
                std::invoke(std::forward<F>(f), Policy::template get<I>(std::forward<T>(t)), I);
            }
            else if constexpr(std::is_invocable_v<func_type, size_t, element_type>)
            {
                std::invoke(std::forward<F>(f), I, Policy::template get<I>(std::forward<T>(t)));
            }
            else
            {
                static_assert(std::disjunction_v<
                    std::is_invocable<func_type, element_type>,
                    std::is_invocable<func_type, element_type, size_t>,
                    std::is_invocable<func_type, size_t, element_type>
                >, "Unsupported function for for_each_field_implement");
            }
        }

        template<typename Policy, typename T, typename F, size_t ... Is>
        inline void for_each_field(Policy policy, T&& t, F&& f, std::index_sequence<Is...>)
        {
            (for_each_field<Is>(policy, std::forward<T>(t), std::forward<F>(f)), ...);
        }

        template<typename Policy, typename T, typename F>
        inline void for_each_field(Policy policy, T&& t, F&& f)
        {
            using index_seq_t = std::make_index_sequence<Policy::template field_count<std::remove_cvref_t<T>>()>;
            for_each_field(policy, std::forward<T>(t), std::forward<F>(f), index_seq_t{});
        }

        template <size_t I, typename Policy, typename T, typename F>
        inline void for_each_field_name(Policy, T&& t, F&& f)
        {
            using type = std::remove_cvref_t<T>;
            using func_type = std::decay_t<std::remove_cvref_t<F>>;
            using element_type = decltype(Policy::template get_name<I, type>());
            if constexpr (std::is_invocable_v<func_type, element_type>)
            {
                std::invoke(std::forward<F>(f), Policy::template get_name<I, type>());
            }
            else if constexpr (std::is_invocable_v<func_type, element_type, size_t>)
            {
                std::invoke(std::forward<F>(f), Policy::template get_name<I, type>(), I);
            }
            else if constexpr (std::is_invocable_v<func_type, size_t, element_type>)
            {
                std::invoke(std::forward<F>(f), I, Policy::template get_name<I, type>());
            }
            else
            {
                static_assert(std::disjunction_v<
                    std::is_invocable<func_type, element_type>,
                    std::is_invocable<func_type, element_type, size_t>,
                    std::is_invocable<func_type, size_t, element_type>
                >, "Unsupported function for for_each_field_implement");
            }
        }

        template<typename Policy, typename T, typename F, size_t ... Is>
        inline void for_each_field_name(Policy policy, T&& t, F&& f, std::index_sequence<Is...>)
        {
            (for_each_field_name<Is>(policy, std::forward<T>(t), std::forward<F>(f)), ...);
        }

        template<typename Policy, typename T, typename F>
        inline void for_each_field_name(Policy policy, T&& t, F&& f)
        {
            using index_seq_t = std::make_index_sequence<Policy::template field_count<std::remove_cvref_t<T>>()>;
            for_each_field_name(policy, std::forward<T>(t), std::forward<F>(f), index_seq_t{});
        }

        template <size_t I, typename Policy, typename T, typename F>
        inline void for_each_field_and_name(Policy, T&& t, F&& f)
        {
            using type = std::remove_cvref_t<T>;
            using func_type = std::decay_t<std::remove_cvref_t<F>>;
            using element_type = decltype(Policy::template get<I>(std::forward<T>(t)));
            using name_type = decltype(Policy::template get_name<I, type>());
            if constexpr (std::is_invocable_v<func_type, element_type, name_type>) // match f(t.*pmd, t.name)
            {
                std::invoke(std::forward<F>(f), Policy::template get<I>(std::forward<T>(t)), Policy::template get_name<I, type>());
            }
            else if constexpr (std::is_invocable_v<func_type, name_type, element_type>) // match f(t.name, t.*pmd)
            {
                std::invoke(std::forward<F>(f), Policy::template get_name<I, type>(), Policy::template get<I>(std::forward<T>(t)));
            }
            else if constexpr (std::is_invocable_v<func_type, size_t, element_type, name_type>) // match f(I, t.*pmd, t.name)
            {
                std::invoke(std::forward<F>(f), I, Policy::template get<I>(std::forward<T>(t)), Policy::template get_name<I, type>());
            }
            else if constexpr (std::is_invocable_v<func_type, size_t, name_type, element_type>) // match f(I, t.name, t.*pmd)
            {
                std::invoke(std::forward<F>(f), I, Policy::template get_name<I, type>(), Policy::template get<I>(std::forward<T>(t)));
            }
            else if constexpr (std::is_invocable_v<func_type, element_type, name_type, size_t>) // match f(t.*pmd, t.name, I)
            {
                std::invoke(std::forward<F>(f), Policy::template get<I>(std::forward<T>(t)), Policy::template get_name<I, type>(), I);
            }
            else if constexpr (std::is_invocable_v<func_type, name_type, element_type, size_t>) // match f(t.name, t.*pmd, I)
            {
                std::invoke(std::forward<F>(f), Policy::template get_name<I, type>(), Policy::template get<I>(std::forward<T>(t)), I);
            }
            else
            {
                static_assert(std::disjunction_v<
                    std::is_invocable<func_type, element_type, name_type>,
                    std::is_invocable<func_type, name_type, element_type>,
                    std::is_invocable<func_type, size_t, element_type, name_type>,
                    std::is_invocable<func_type, size_t, name_type, element_type>,
                    std::is_invocable<func_type, element_type, name_type, size_t>,
                    std::is_invocable<func_type, name_type, element_type, size_t>
                >, "Unsupported function for for_each_field_implement");
            }
        }

        template <typename Policy, typename T, typename F, size_t ... Is>
        inline void for_each_field_and_name(Policy policy, T&& t, F&& f, std::index_sequence<Is...>)
        {
            (for_each_field_and_name<Is>(policy, std::forward<T>(t), std::forward<F>(f)), ...);
        }

        template<typename Policy, typename T, typename F>
        inline void for_each_field_and_name(Policy policy, T&& t, F&& f)
        {
            using index_seq_t = std::make_index_sequence<Policy::template field_count<std::remove_cvref_t<T>>()>;
            for_each_field_and_name(policy, std::forward<T>(t), std::forward<F>(f), index_seq_t{});
        }
    }

    template <typename T, typename F> requires reflectable<std::remove_cvref_t<T>>
    inline void for_each_field(T&& t, F&& f)
    {
        if constexpr(reflected<std::remove_cvref_t<T>>)
        {
            detail::for_each_field(reflected_policy{}, std::forward<T>(t), std::forward<F>(f));
        }
        else
        {
            static_assert(auto_reflectable<std::remove_cvref_t<T>>);
            detail::for_each_field(auto_reflectable_policy{}, std::forward<T>(t), std::forward<F>(f));
        }
    }

    template <typename T, typename F> requires reflectable<std::remove_cvref_t<T>>
    inline void for_each_field_name(T&& t, F&& f)
    {
        if constexpr(reflected<std::remove_cvref_t<T>>)
        {
            detail::for_each_field_name(reflected_policy{}, std::forward<T>(t), std::forward<F>(f));
        }
        else
        {
            static_assert(auto_reflectable<std::remove_cvref_t<T>>);
            detail::for_each_field_name(auto_reflectable_policy{}, std::forward<T>(t), std::forward<F>(f));
        }
    }

    template <typename T, typename F> requires reflectable<std::remove_cvref_t<T>>
    inline void for_each_field_and_name(T&& t, F&& f)
    {
        if constexpr (reflected<std::remove_cvref_t<T>>)
        {
            detail::for_each_field_and_name(reflected_policy{}, std::forward<T>(t), std::forward<F>(f));
        }
        else
        {
            static_assert(auto_reflectable<std::remove_cvref_t<T>>);
            detail::for_each_field_and_name(auto_reflectable_policy{}, std::forward<T>(t), std::forward<F>(f));
        }
    }
}