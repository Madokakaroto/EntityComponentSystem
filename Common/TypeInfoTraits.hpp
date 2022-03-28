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

    #define ECS_IMPLEMENT_PRIMATIVE_TYPE(CppType, TypeName)                 \
    template <>                                                             \
    struct type_info_traits<CppType> : primative_type_info_traits<CppType>  \
    {                                                                       \
        using type = typename primative_type_info_traits<CppType>::type;    \
        static std::string get_type_name()                                  \
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
}