#pragma once
#include "boost/preprocessor.hpp"
#include "boost/pfr.hpp"
#include "Traits/TypeTraitsExt.hpp"
#include "Types/Forward.hpp"

namespace punk
{
    template <typename T>
    struct reflect_info;

    template <typename T>
    concept reflected = requires
    {
        // reflect_info shall be a complete type
        std::is_trivially_constructible<reflect_info<T>>;
        std::is_trivially_default_constructible<reflect_info<T>>;

        // field count
        { reflect_info<T>::field_count() } -> std::convertible_to<size_t>;

        // members
        { reflect_info<T>::members() };

        // member names
        { reflect_info<T>::member_names() };
    };

    template <typename T>
    concept auto_reflectable = boost::pfr::is_reflectable<T>::value;

    template <typename T>
    concept reflectable = reflected<T> || auto_reflectable<T>;
}

// implementation detail for PUNK_REFLECT_MEMBERS
#define PUNK_REFLECT_APPLY_MACRO(macro) macro
#define PUNK_REFLECT_ACCESS_MEMBER(c, m) &c::m
#define PUNK_REFLECT_MEMBER_NAME_II(member) std::string_view{ #member }
#define PUNK_REFLECT_MEMBER_NAME_I(member) PUNK_REFLECT_MEMBER_NAME_II(member)
#define PUNK_REFLECT_MEMBER_NAME(r, data, i, t) BOOST_PP_COMMA_IF(i) PUNK_REFLECT_MEMBER_NAME_I(t)

// members generation for reflect_info
#define PUNK_REFLECT_MEMBER_DATA(r, data, i, t) BOOST_PP_COMMA_IF(i) PUNK_REFLECT_ACCESS_MEMBER(data, t)
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
}

// the reflection interface
#define PUNK_REFLECT(CLASS, ...)                                                  \
template <> struct ::punk::reflect_info<CLASS>                                    \
{                                                                                 \
    static_assert(std::negation_v<std::junction<                                  \
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
        template <std::size_t I, typename T, typename F, typename Tuple> requires reflected<std::remove_cvref_t<T>>
        inline void for_each_field_implement(T&& t, F&& f, Tuple const& tuple)
        {
            // decay the function type
            using func_type = std::decay_t<std::remove_cvref_t<F>>;

            // export the type in tuple with index of I
            using tuple_element_type = std::tuple_element_t<I, Tuple>;

            // when the element type if pointer-to-member-data, export the acutal field type in the T, otherwise export tuple_element_type
            using element_type = std::conditional_t<
                std::is_member_function_pointer_v<tuple_element_type>,
                decltype(std::forward<T>(t).*std::get<I>(tuple)), tuple_element_type>;

            // match f(field)
            if constexpr(std::is_invocable_v<func_type, element_type>)
            {
                // match f(t.field)
                if constexpr(std::is_member_function_pointer_v<tuple_element_type>)
                {
                    std::forward<F>(f)(std::forward<T>(t).*std::get<I>(tuple));
                }
                else // match f(t.field_name)
                {
                    std::forward<F>(f)(std::get<I>(tuple));
                }
            }
            // match f(field, Index)
            else if constexpr (std::is_invocable_v<func_type, element_type, size_t>)
            {
                // match f(t.field, Index)
                if constexpr (std::is_member_function_pointer_v<tuple_element_type>)
                {
                    std::forward<F>(f)(std::forward<T>(t).*std::get<I>(tuple), I);
                } // match f(t.field_name, Index);
                else
                {
                    std::forward<F>(f)(std::get<I>(tuple), I);
                }
            }
            // match f(Index, field)
            else if constexpr (std::is_invocable_v<func_type, size_t, element_type>)
            {
                // match f(Index, t.field)
                if constexpr (std::is_member_function_pointer_v<tuple_element_type>)
                {
                    std::forward<F>(f)(I, std::forward<T>(t).*std::get<I>(tuple));
                }
                else // match f(Index, t.field_name);
                {
                    std::forward<F>(f)(I, std::get<I>(tuple));
                }
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

        template <typename T, typename F, typename Tuple, std::size_t ... Is>
        inline void for_each_field_implement(T&& t, F&& f, Tuple const& tuple, std::index_sequence<Is...>)
        {
            (for_each_field_implement<Is>(std::forward<T>(t), std::forward<F>(f), tuple), ...);
        }

        template <std::size_t I, typename T, typename F, typename PmdTuple, typename NameTuple> requires reflected<std::remove_cvref_t<T>>
        inline void for_each_field_implement(T&& t, F&& f, PmdTuple const& pmd_tuple, NameTuple const& name_tuple)
        {
            using func_type = std::decay_t<std::remove_cvref_t<F>>;
            using element_type = decltype(std::forward<T>(t).*std::get<I>(pmd_tuple));
            using name_type = decltype(std::get<I>(name_tuple));
            if constexpr (std::is_invocable_v<func_type, element_type, name_type>) // match f(t.*pmd, t.name)
            {
                std::forward<F>(f)(std::forward<T>(t).*std::get<I>(pmd_tuple), std::get<I>(name_tuple));
            }
            else if constexpr(std::is_invocable_v<func_type, name_type, element_type>) // match f(t.name, t.*pmd)
            {
                std::forward<F>(f)(std::get<I>(name_tuple), std::forward<T>(t).*std::get<I>(pmd_tuple));
            }
            else if constexpr(std::is_invocable_v<func_type, size_t, element_type, name_type>) // match f(I, t.*pmd, t.name)
            {
                std::forward<F>(f)(I, std::forward<T>(t).*std::get<I>(pmd_tuple), std::get<I>(name_tuple));
            }
            else if constexpr (std::is_invocable_v<func_type, size_t, name_type, element_type>) // match f(I, t.name, t.*pmd)
            {
                std::forward<F>(f)(I, std::get<I>(name_tuple), std::forward<T>(t).*std::get<I>(pmd_tuple));
            }
            else if constexpr (std::is_invocable_v<func_type, element_type, name_type, size_t>) // match f(t.*pmd, t.name, I)
            {
                std::forward<F>(f)(std::forward<T>(t).*std::get<I>(pmd_tuple), std::get<I>(name_tuple), I);
            }
            else if constexpr (std::is_invocable_v<func_type, name_type, element_type, size_t>) // match f(t.name, t.*pmd, I)
            {
                std::forward<F>(f)(std::get<I>(name_tuple), std::forward<T>(t).*std::get<I>(pmd_tuple), I);
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

        template <typename T, typename F, typename PmdTuple, typename NameTuple, std::size_t ... Is>
        inline void for_each_field_implement(T&& t, F&& f, PmdTuple const& pmd_tuple, NameTuple const& name_tuple, std::index_sequence<Is...>)
        {
            (for_each_field_implement<Is>(std::forward<T>(t), std::forward<F>(f), pmd_tuple, name_tuple), ...);
        }
    }

    template <reflectable T, typename F>
    inline void for_each_field(T&& t, F&& f)
    {
        using reflect_info_t = reflect_info<std::remove_cvref_t<T>>;
        detail::for_each_field_implement(std::forward<T>(t), std::forward<F>(f),
            reflect_info_t::members(), std::make_index_sequence<reflect_info_t::field_count()>);
    }

    template <reflectable T, typename F>
    inline void for_each_field_name(T&& t, F&& f)
    {
        using reflect_info_t = reflect_info<std::remove_cvref_t<T>>;
        detail::for_each_field_implement(std::forward<T>(t), std::forward<F>(f),
            reflect_info_t::member_names(), std::make_index_sequence<reflect_info_t::field_count()>);
    }

    template <reflectable T, typename F>
    inline void for_each_field_and_name(T&& t, F&& f)
    {
        using reflect_info_t = reflect_info<std::remove_cvref_t<T>>;
        detail::for_each_field_implement(std::forward<T>(t), std::forward<F>(f),
            reflect_info_t::members(), reflect_info_t::member_names(),
            std::make_index_sequence<reflect_info_t::field_count()>);
    }
}