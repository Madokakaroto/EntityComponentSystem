#pragma once

#include <format>
#include "Traits/TypeTraitsExt.hpp"
#include "Utils/TypeDemangle.hpp"
#include "Utils/Hash.hpp"
#include "Types/Meta.h"
#include "Utils/StaticReflection.hpp"

namespace punk
{
    // generic traits implementation
    template <typename T>
    struct type_info_traits;
}

// for primative types
namespace punk
{
    template <typename T>
    struct primative_type_info_traits
    {
        using type = T;

        static std::string get_type_name()
        {
            return get_demangle_name<type>();
        }

        static constexpr size_t get_field_count() noexcept
        {
            return size_t{ 0 };
        }

        static uint32_t get_hash()
        {
            auto const type_name = type::get_type_name();
            return hash_memory(type_name.c_str(), type_name.length());
        }

        static auto get_vtable() noexcept -> type_vtable_t
        {
            type_vtable_t vtable = { nullptr, nullptr, nullptr, nullptr, nullptr };

            if constexpr(std::negation_v<std::is_trivially_constructible<T>>)
            {
                vtable.constructor = [](void* addr) { new (addr) T{}; };
            }

            if constexpr(std::negation_v<std::is_trivially_destructible<T>>)
            {
                vtable.destructor = [](void* addr) { reinterpret_cast<T*>(addr)->~T(); };
                vtable.copy_func = [](void* dst, void const* src) { *reinterpret_cast<T*>(dst) = *reinterpret_cast<T const*>(src); };
                vtable.swap_func = [](void* lhs, void* rhs) { std::swap(*reinterpret_cast<T*>(lhs), *reinterpret_cast<T*>(rhs)); };
                vtable.move_func = [](void* dst, void* src) { *reinterpret_cast<T*>(dst) = std::move(*reinterpret_cast<T const*>(src)); };
            }
        }
    };

    #define PUNK_IMPLEMENT_PRIMATIVE_TYPE(Type, TypeName)                   \
    template <>                                                             \
    struct type_info_traits<Type> : primative_type_info_traits<Type>        \
    {                                                                       \
        using type = typename primative_type_info_traits<Type>::type;       \
        static std::string get_type_name()                                  \
        {                                                                   \
            return #TypeName;                                               \
        }                                                                   \
    }

    PUNK_IMPLEMENT_PRIMATIVE_TYPE(bool, bool);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(uint8_t, uint8);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(sint8_t, sint8);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(uint16_t, uint16);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(sint16_t, sint16);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(uint32_t, uint32);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(sint32_t, sint32);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(float, float);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(double, double);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(string, std::string);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(wstring, std::wstring);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(u8string, std::u8string);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(u16string, std::u16string);
    PUNK_IMPLEMENT_PRIMATIVE_TYPE(u32string, std::u32string);

    template <typename T, size_t Size>
    struct type_info_traits<std::array<T, Size>> : 
        primative_type_info_traits<std::array<T, Size>>
    {
        using type = typename primative_type_info_traits<std::array<T, Size>>::type;

        static std::string get_type_name()
        {
            using value_type_info = type_info_traits<T>;
            return std::format("std::array<{}, {}>", value_type_info::get_type_name(), Size);
        }
    };

    template <typename T>
    struct type_info_traits<vector<T>> : primative_type_info_traits<vector<T>>
    {
        using type = typename primative_type_info_traits<vector<T>>::type;

        static std::string get_type_name()
        {
            using value_type_info = type_info_traits<T>;
            return std::format("std::vector<{}>", value_type_info::get_type_name());
        }
    };

    template <typename Key, typename Value>
    struct type_info_traits<map<Key, Value>> : primative_type_info_traits<map<Key, Value>>
    {
        using type = typename primative_type_info_traits<map<Key, Value>>::type;

        static std::string get_type_name()
        {
            using key_type_info = type_info_traits<Key>;
            using value_type_info = type_info_traits<Value>;
            return std::format("std::map<{}, {}>", key_type_info::get_type_info(), value_type_info::get_type_name());
        }
    };

    template <typename Key, typename Value>
    struct type_info_traits<unordered_map<Key, Value>> : primative_type_info_traits<unordered_map<Key, Value>>
    {
        using type = typename primative_type_info_traits<unordered_map<Key, Value>>::type;

        static std::string get_type_name()
        {
            using key_type_info = type_info_traits<Key>;
            using value_type_info = type_info_traits<Value>;
            return std::format("std::unordered_map<{}, {}>", key_type_info::get_type_info(), value_type_info::get_type_name());
        }
    };
}

// for reflectible types
namespace punk
{
    template <reflected T>
    struct type_info_traits<T> : primative_type_info_traits<T>
    {
        using type = typename primative_type_info_traits<T>::type;
        using reflect_info_t = reflect_info<type>;

        static constexpr size_t field_count() noexcept
        {
            return reflect_info_t::field_count();
        }

        template <size_t I>
        static constexpr auto field_type() noexcept -> tuple_element_t<I, T>;

        template <size_t I>
        static /*constexpr*/ size_t field_offset() noexcept
        {
            return std::get<I>(reflect_info_t::member_offsets());
        }
    };

    namespace detail
    {
        template <typename Tuple>
        struct tuple_to_sequence_tuple;
        template <typename ... Args>
        struct tuple_to_sequence_tuple<std::tuple<Args...>>
        {
            using type = boost::pfr::detail::sequence_tuple::tuple<Args...>;
        };
        template <typename Tuple>
        using tuple_to_sequence_tuple_t = typename tuple_to_sequence_tuple<Tuple>::type;
    }

    template <typename T> requires auto_reflectable<T> && !reflected<T>
    struct type_info_traits<T> : primative_type_info_traits<T>
    {
        using type = typename primative_type_info_traits<T>::type;

        static constexpr size_t field_count() noexcept
        {
            return boost::pfr::tuple_size_v<type>;
        }

        template <size_t I>
        static constexpr auto field_type() noexcept -> boost::pfr::tuple_element_t<I, type>;

        template <size_t I>
        static /*constexpr*/ size_t field_offset() noexcept
        {
            return reinterpret_cast<size_t>(std::addressof(boost::pfr::get<I>(*reinterpret_cast<type*>(0))));
        }
    };
}