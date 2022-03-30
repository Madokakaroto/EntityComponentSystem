#pragma once

#include <type_traits>
#include <format>
#include "TypeTraitsExt.hpp"

namespace ecs
{
    template <typename T>
    struct primative_type_info_traits
    {
        using type = T;

        [[nodiscard]] static std::string get_type_name()
        {
            return get_demangle_name<type>();
        }

        [[nodiscard]] static constexpr size_t get_field_count() noexcept
        {
            return size_t{ 0 };
        }

        [[nodiscard]] static uint32_t get_hash()
        {
            auto const type_name = type::get_type_name();
            return hash_memory(type_name.c_str(), type_name.length());
        }

        [[nodiscard]] static auto get_vtable() noexcept -> archetype_vtable_t
        {
            archetype_vtable_t vtable = { nullptr, nullptr, nullptr, nullptr };

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

    // generic traits implementation
    template <typename T>
    struct type_info_traits : primative_type_info_traits<T>
    {
    public: 
        // static type check
        static_assert(std::conjunction_v<
            std::negation<std::is_reference<T>>,
            std::negation<std::is_pointer<T>>,
            std::negation<std::is_const<T>>,
            std::negation<std::is_volatile<T>>,
            std::negation<std::is_function<T>>,
            std::negation<std::is_void<T>>,
            std::negation<is_char_array<T>>
            >, "Type not Supported!"
        );

        using type = typename primative_type_info_traits<T>::type;
    };

    #define ECS_IMPLEMENT_PRIMATIVE_TYPE(Type, TypeName)                    \
    template <>                                                             \
    struct type_info_traits<Type> : primative_type_info_traits<Type>        \
    {                                                                       \
        using type = typename primative_type_info_traits<Type>::type;       \
        [[nodiscard]] static std::string get_type_name()                    \
        {                                                                   \
            return #TypeName;                                               \
        }                                                                   \
    }

    ECS_IMPLEMENT_PRIMATIVE_TYPE(bool, bool);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(uint8_t, uint8);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(sint8_t, sint8);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(uint16_t, uint16);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(sint16_t, sint16);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(uint32_t, uint32);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(sint32_t, sint32);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(float, float);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(double, double);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(string, std::string);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(wstring, std::wstring);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(u8string, std::u8string);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(u16string, std::u16string);
    ECS_IMPLEMENT_PRIMATIVE_TYPE(u32string, std::u32string);

    template <typename T, size_t Size>
    struct type_info_traits<std::array<T, Size>> : 
        primative_type_info_traits<std::array<T, Size>>
    {
        using type = typename primative_type_info_traits<std::array<T, Size>>::type;

        [[nodiscard]] static std::string get_type_name()
        {
            using value_type_info = type_info_traits<T>;
            return std::format("std::array<{}, {}>", value_type_info::get_type_name(), Size);
        }
    };

    template <typename T>
    struct type_info_traits<vector<T>> : primative_type_info_traits<vector<T>>
    {
        using type = typename primative_type_info_traits<vector<T>>::type;

        [[nodiscard]] static std::string get_type_name()
        {
            using value_type_info = type_info_traits<T>;
            return std::format("std::vector<{}>", value_type_info::get_type_name());
        }
    };

    template <typename Key, typename Value>
    struct type_info_traits<map<Key, Value>> : primative_type_info_traits<map<Key, Value>>
    {
        using type = typename primative_type_info_traits<map<Key, Value>>::type;

        [[nodiscard]] static std::string get_type_name()
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

        [[nodiscard]] static std::string get_type_name()
        {
            using key_type_info = type_info_traits<Key>;
            using value_type_info = type_info_traits<Value>;
            return std::format("std::unordered_map<{}, {}>", key_type_info::get_type_info(), value_type_info::get_type_name());
        }
    };
}