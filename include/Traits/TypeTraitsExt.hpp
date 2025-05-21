#pragma once

#include <type_traits>

// std extension
namespace punk
{
    template <bool Test, typename T = void>
    using disable_if = std::enable_if<!Test, T>;
    template <bool Test, typename T = void>
    using disable_if_t = typename disable_if<Test, T>::type;
}

#define PUNK_TRAITS_MEMBER_TYPE(member)                         \
template <typename T>                                           \
concept has_##member = requires { typename T::member; };        \
template <typename T>                                           \
struct traits_##member                                          \
{                                                               \
    using type = void;                                          \
};                                                              \
template <typename T> requires has_##member<T>                  \
struct traits_##member<T>                                       \
{                                                               \
    using type = typename T::member;                            \
};                                                              \
template <typename T>                                           \
using traits_##member##_t = typename traits_##member<T>::type;

namespace punk
{
    PUNK_TRAITS_MEMBER_TYPE(element_type);
    PUNK_TRAITS_MEMBER_TYPE(value_type);
    PUNK_TRAITS_MEMBER_TYPE(attributes);

    template <typename T, typename S>
    constexpr auto owner_type_of_pmd(T S::*) noexcept -> T;
}

// boolean meta-functions
namespace punk
{
    template <typename T>
    struct is_bool : std::is_same<std::remove_cv_t<T>, bool> {};
    template <typename T>
    constexpr bool is_bool_v = is_bool<T>::value;

    template <typename T>
    using is_strict_integral = std::conjunction<
        std::is_integral<T>,
        std::negation<is_bool<T>>
    >;
    template <typename T>
    constexpr bool is_strict_integral_v = is_strict_integral<T>::value;
    template <typename T>
    concept strict_integral = is_strict_integral_v<T>;

    template <typename T, typename P>
    using is_pointer_of = std::conjunction<
        std::is_pointer<T>, 
        std::is_same<std::remove_cv_t<std::remove_pointer_t<T>>, P>
    >;
    template <typename T, typename P>
    constexpr bool is_pointer_of_v = is_pointer_of<T, P>::value;
    template <typename T, typename P>
    concept pointer_of = is_pointer_of_v<T, P>;

    template <typename T>
    using is_cstring = std::disjunction<
        is_pointer_of<T, char>,
        is_pointer_of<T, wchar_t>,
        is_pointer_of<T, char8_t>,
        is_pointer_of<T, char16_t>,
        is_pointer_of<T, char32_t>
    >;
    template <typename T>
    constexpr bool is_cstring_v = is_cstring<T>::value;
    template <typename T>
    concept cstring = is_cstring_v<T>;

    template <typename T>
    concept std_string = requires(T str)
    {
        { str.size() } -> std::convertible_to<std::size_t>;
        { str.c_str() } -> cstring;
        { str.length() } -> std::convertible_to<std::size_t>;
    };
    template <typename T>
    constexpr bool is_std_string_v = std_string<T>;
    template <typename T>
    using is_std_string = std::bool_constant<is_std_string_v<T>>;

    template <typename T, typename ... Args>
    using is_among_types = std::disjunction<std::is_same<T, Args>...>;
    template <typename T, typename ... Args>
    constexpr bool is_among_types_v = is_among_types<T, Args...>::value;
    template <typename T, typename ... Args>
    concept among_types = is_among_types_v<T, Args...>;

    template <typename T>
    using is_char_array = std::conjunction<
        std::is_array<T>,
        is_among_types<T, char, wchar_t, char8_t, char16_t, char32_t>,
        std::bool_constant<std::rank_v<T> == 1>
    >;
    template <typename T>
    constexpr bool is_char_array_v = is_char_array<T>::value;
    template <typename T>
    concept char_array = is_char_array_v<T>;

    template <typename T>
    concept enumerable = std::is_enum_v<T>;

    template <typename T>
    concept smart_pointer = requires(T ptr)
    {
        { *ptr } -> std::convertible_to<T>;
        { ptr.get() } -> std::convertible_to<T>;
        { static_cast<bool>(ptr) };
    };

    template <typename T>
    concept sizeofable = requires
    {
        { sizeof(T) } -> std::convertible_to<size_t>;
    };
    template <typename T>
    concept incomplete_type = !sizeofable<T>;

    template <typename ... Args>
    struct tuple_has_repeated_types;
    template <typename T>
    struct tuple_has_repeated_types<T> : std::false_type{};
    template <typename T, typename ... Args>
    struct tuple_has_repeated_types<T, T, Args...> : std::true_type{};
    template <typename T, typename P, typename ... Args>
    struct tuple_has_repeated_types<T, P, Args...> : std::bool_constant<std::disjunction_v<
        tuple_has_repeated_types<T, Args...>, tuple_has_repeated_types<P, Args...> >>{};
    template <typename ... Args>
    constexpr bool tuple_has_repeated_types_v = tuple_has_repeated_types<Args...>::value;
}